#include <stdio.h>
#include <stdlib.h>
#include <usb.h>

#define USB_TIMEOUT              5000
#define CAMERA_BULK_READ_EP      0x81
#define CAMERA_BULK_WRITE_EP     0x02
#define CAMERA_INT_READ_EP       0x83

#define uint8_t unsigned char

#define htole32a(a,x) (a)[3]=(unsigned char)((x)>>24), \
                      (a)[2]=(unsigned char)((x)>>16), \
                      (a)[1]=(unsigned char)((x)>>8), \
                      (a)[0]=(unsigned char)(x)

void print_usb_info(void)
{
  struct usb_bus *bus;
  struct usb_device *device;
  struct usb_config_descriptor *config;
  struct usb_interface *interface;
  struct usb_interface_descriptor *interface_setting;
  struct usb_endpoint_descriptor *endpoint;
  int i, j;

  bus = usb_busses;
  while(bus) {
    fprintf(stderr, "Found bus %s\n", bus->dirname);
    
    device = bus->devices;
    while(device) {
      fprintf(stderr, "  Found device %s  (Vendor 0x%04x Product 0x%04x)\n", 
	      device->filename, device->descriptor.idVendor,
	      device->descriptor.idProduct);
      
      interface = device->config->interface;
      for(i = 0; i < interface->num_altsetting; i++) {
	interface_setting = &(interface->altsetting[i]);
	fprintf(stderr, "    Found interface setting %d - "
		"number %d - %d enpoints\n", i,
		interface_setting->bInterfaceNumber,
		interface_setting->bNumEndpoints);
	
	for(j = 0; j < interface_setting->bNumEndpoints; j++) {
	  endpoint = &(interface_setting->endpoint[j]);
	  fprintf(stderr, "      Endpoint %d - address %d - direction %s\n", j,
		  endpoint->bEndpointAddress & USB_ENDPOINT_ADDRESS_MASK,
		  (endpoint->bEndpointAddress & USB_ENDPOINT_DIR_MASK) ? "IN" 
		  : "OUT");
	}
      }
      device = device->next;
    }
    bus = bus->next;
  }
}

struct usb_device *find_usb_device(int vendor, int product)
{
  struct usb_bus *bus;
  struct usb_device *device;

  bus = usb_busses;
  while(bus) {
    device = bus->devices;
    while(device) {
      if(device->descriptor.idVendor == vendor &&
	 device->descriptor.idProduct == product)
	return device;
      device = device->next;
    }
    bus = bus->next;
  }
  return NULL;
}

int camera_control_read(usb_dev_handle *handle, int request, int value,
			int index, char *buffer, int length)
{
  fprintf(stderr, "CAMERA CONTROL READ %x\n", length);
  return usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | 
			 USB_ENDPOINT_IN, request, value, index, 
			 buffer, length, USB_TIMEOUT);
}

int camera_control_write(usb_dev_handle *handle, int request, int value,
			 int index, char *buffer, int length)
{
  fprintf(stderr, "CAMERA CONTROL WRITE %x\n", length);
  return usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE,
			 request, value, index, buffer, length, USB_TIMEOUT);
}

int camera_bulk_read(usb_dev_handle *handle, char *buffer, int length)
{
  fprintf(stderr, "CAMERA BULK READ %x\n", length);
  return usb_bulk_read(handle, CAMERA_BULK_READ_EP, buffer, length, 
		       USB_TIMEOUT);
}

int camera_bulk_write(usb_dev_handle *handle, char *buffer, int length)
{
  fprintf(stderr, "CAMERA BULK READ %x\n", length);
  return usb_bulk_write(handle, CAMERA_BULK_WRITE_EP, buffer, length,
			USB_TIMEOUT);
}

int camera_int_read(usb_dev_handle *handle, char *buffer, int length)
{
  fprintf(stderr, "CAMERA INT READ %x\n", length);
  return usb_bulk_read(handle, CAMERA_INT_READ_EP, buffer, length, 50);
}

int camera_int_read_block(usb_dev_handle *handle, char *buffer, int length)
{
  int l;

  while((l = camera_int_read(handle, buffer, length)) < length);
  return l;
}

int canon_initialize_camera(usb_dev_handle *camera_handle)
{
  unsigned char msg[0x58], buffer[0x44];
  unsigned char camstat;
  int i;

  memset(msg, 0, sizeof(msg));
  memset(buffer, 0, sizeof(buffer));

  i = camera_control_read(camera_handle, 0x0c, 0x55, 0x00, msg, 0x01);
  camstat = msg[0];
  if(i != 1 || (camstat != 'A' && camstat != 'C')) {
    fprintf(stderr, "Error: could not initialize camera - step 1\n");
    return -1;
  }
  
  i = camera_control_read(camera_handle, 0x04, 0x01, 0x00, msg, 0x58);
  if(i != 0x58) {
    fprintf(stderr, "Error: could not initialize camera - step 2\n");
    return -1;
  }

  if(camstat == 'A') {
    i = camera_control_read(camera_handle, 0x04, 0x04, 0x00, msg, 0x50);
    if(i != 0x50) {
      fprintf(stderr, "Error: could not initialize camera - step 3\n");
      return -1;
    }
    return 0;
  }
  
  msg[0] = 0x10;
  memmove(msg + 0x40, msg + 0x48, 0x10);
  i = camera_control_write(camera_handle, 0x04, 0x11, 0x00, msg, 0x50);
  if(i != 0x50) {
    fprintf(stderr, "Error: could not initialize camera - step 3\n");
    return -1;
  }

  i = camera_bulk_read(camera_handle, buffer, 0x40);
  if((i >= 4) && (buffer[i - 4] == 0x54) && (buffer[i - 3] == 0x78) &&
     (buffer[i - 2] == 0x00) && (buffer[i - 1] == 0x00))
    return 0;

  if(i != 0x40) {
    fprintf(stderr, "Error: could not initialize camera - step 4\n");
    return -1;
  }
  
  i = camera_bulk_read(camera_handle, buffer, 0x04);
  if(i != 4) {
    fprintf(stderr, "Error: could not initialize camera - step 4\n");
    return -1;
  }
  i = camera_int_read_block(camera_handle, buffer, 0x10);
  if(i != 0x10) {
    fprintf(stderr, "Error: could not initialize camera. int step\n");
    return -1;
  }
  return 0;
}

unsigned char *canon_fill_command(int length, int cmd1, unsigned char cmd2, 
				  unsigned char cmd3, unsigned char *payload,
				  int payload_length)
{
  unsigned char *buffer;
  
  buffer = (unsigned char *)calloc(length, 1);
  memset(buffer, 0, length);
  htole32a(buffer, 0x10 + payload_length);
  htole32a(buffer + 0x04, cmd1);
  buffer[0x40] = 2;
  buffer[0x44] = cmd2;
  buffer[0x47] = cmd3;
  htole32a(buffer + 0x48, 0x10 + payload_length);
  htole32a(buffer + 0x4c, 0x12345678);
  if(payload != NULL)
    memcpy(buffer + 0x50, payload, payload_length);
  return buffer;
}

unsigned char *canon_fill_payload(int length, int word1, int word2, int word3, 
				  int word4)
{
  unsigned char *buffer;
  
  buffer = (unsigned char *)calloc(length, 1);
  htole32a(buffer, word1);
  if(length >= 8)
    htole32a(buffer + 4, word2);
  if(length >= 12)
    htole32a(buffer + 8, word3);
  if(length >= 16)
    htole32a(buffer + 12, word4);
  return buffer;
}

void canon_print_command(unsigned char *command, int length, char *name)
{
  int i;

  printf("%s:\n", name);
  for(i = 0; i < length; i++) {
    printf("%02x ", command[i]);
    if(i > 0 && (i + 1) % 16 == 0)
      printf("\n");
  }
  printf("\n");
}

int canon_command_identify_camera(usb_dev_handle *camera_handle)
{
  unsigned char *command, response[0x9c];
  int l;

  command = canon_fill_command(0x50, 0x201, 0x01, 0x12, NULL, 0);
  l = camera_control_write(camera_handle, 0x04, 0x10, 0x0, command, 0x50);
  if(l != 0x50)
    return -1;
  l = camera_bulk_read(camera_handle, response, 0x80);
  if(l != 0x80)
    return -1;
  l = camera_bulk_read(camera_handle, response + 0x80, 0x1c);
  if(l != 0x1c)
    return -1;
  free(command);
  return 0;
}

int canon_command_get_picture_abilities(usb_dev_handle *camera_handle)
{
  unsigned char *command, response[0x384];
  int l;

  command = canon_fill_command(0x50, 0x201, 0x1f, 0x12, NULL, 0);
  l = camera_control_write(camera_handle, 0x04, 0x10, 0x0, command, 0x50);
  if(l != 0x50)
    return -1;
  l = camera_bulk_read(camera_handle, response, 0x380);
  if(l != 0x380)
    return -1;
  l = camera_bulk_read(camera_handle, response + 0x380, 0x4);
  if(l != 0x4)
    return -1;
  free(command);
  return 0;
}

int canon_command_lock_keys(usb_dev_handle *camera_handle)
{
  unsigned char *command, response[0x54];
  int l;

  command = canon_fill_command(0x50, 0x201, 0x20, 0x12, NULL, 0);
  l = camera_control_write(camera_handle, 0x04, 0x10, 0x0, command, 0x50);
  if(l != 0x50)
    return -1;
  l = camera_bulk_read(camera_handle, response, 0x40);
  if(l != 0x40)
    return -1;
  l = camera_bulk_read(camera_handle, response + 0x40, 0x14);
  if(l != 0x14)
    return -1;
  free(command);
  return 0;
}

int canon_command_get_power_status(usb_dev_handle *camera_handle)
{
  unsigned char *command, response[0x58];
  int l;

  command = canon_fill_command(0x50, 0x201, 0x0a, 0x12, NULL, 0);
  l = camera_control_write(camera_handle, 0x04, 0x10, 0x0, command, 0x50);
  if(l != 0x50)
    return -1;
  l = camera_bulk_read(camera_handle, response, 0x40);
  if(l != 0x40)
    return -1;
  l = camera_bulk_read(camera_handle, response + 0x40, 0x18);
  if(l != 0x18)
    return -1;
  free(command);
  return 0;
}

int canon_command_rcc_init(usb_dev_handle *camera_handle)
{
  unsigned char *command, *payload, response[0x5c];
  int l;

  payload = canon_fill_payload(0x08, 0, 0, 0, 0);
  command = canon_fill_command(0x58, 0x201, 0x13, 0x12, payload, 0x08);
  l = camera_control_write(camera_handle, 0x04, 0x10, 0x0, command, 0x58);
  if(l != 0x58)
    return -1;
  l = camera_bulk_read(camera_handle, response, 0x40);
  if(l != 0x40)
    return -1;
  l = camera_bulk_read(camera_handle, response + 0x40, 0x1c);
  if(l != 0x1c)
    return -1;
  free(command);
  free(payload);
  return 0;
}

int canon_command_rcc_unknown(usb_dev_handle *camera_handle)
{
  unsigned char *command, *payload, response[0x5c];
  int l;

  payload = canon_fill_payload(0xc, 9, 4, (1 << 24) | 11, 0);
  command = canon_fill_command(0x5c, 0x201, 0x13, 0x12, payload, 0xc);
  l = camera_control_write(camera_handle, 0x04, 0x10, 0x0, command, 0x5c);
  if(l != 0x5c)
    return -1;
  l = camera_bulk_read(camera_handle, response, 0x40);
  if(l != 0x40)
    return -1;
  l = camera_bulk_read(camera_handle, response + 0x40, 0x1c);
  if(l != 0x1c)
    return -1;
  free(command);
  free(payload);
  return 0;
}

int canon_command_rcc_set_transfer_mode(usb_dev_handle *camera_handle)
{
  unsigned char *command, *payload, response[0x5c];
  int l;

  payload = canon_fill_payload(0xc, 9, 4, 11, 0);
  command = canon_fill_command(0x5c, 0x201, 0x13, 0x12, payload, 0xc);
  l = camera_control_write(camera_handle, 0x04, 0x10, 0x0, command, 0x5c);
  if(l != 0x5c)
    return -1;
  l = camera_bulk_read(camera_handle, response, 0x40);
  if(l != 0x40)
    return -1;
  l = camera_bulk_read(camera_handle, response + 0x40, 0x1c);
  if(l != 0x1c)
    return -1;
  free(command);
  free(payload);
  return 0;
}

int canon_command_rcc_release_shutter(usb_dev_handle *camera_handle)
{
  unsigned char *command, *payload, response[0x5c];
  int l;

  payload = canon_fill_payload(0x08, 4, 0, 0, 0);
  command = canon_fill_command(0x58, 0x201, 0x13, 0x12, payload, 0x8);
  
  fprintf(stderr, "before snap\n");
  l = camera_control_write(camera_handle, 0x04, 0x10, 0x0, command, 0x58);
  if(l != 0x58)
    return -1;
  l = camera_bulk_read(camera_handle, response, 0x40);
  if(l != 0x40)
    return -1;
  l = camera_bulk_read(camera_handle, response + 0x40, 0x1c);
  if(l != 0x1c)
    return -1;
  
  fprintf(stderr, "got here 1\n");
  l = camera_int_read_block(camera_handle, response, 0x10);
  fprintf(stderr, "got here 2\n");
  l = camera_int_read_block(camera_handle, response, 0x17);
  fprintf(stderr, "got here 3\n");
  l = camera_int_read_block(camera_handle, response, 0x17);
  fprintf(stderr, "got here 4\n");
  l = camera_int_read_block(camera_handle, response, 0x10);
  fprintf(stderr, "got here 5\n");

  free(command);
  free(payload);
  return 0;
}

int canon_command_download_captured_thumbnail(usb_dev_handle *camera_handle)
{
  unsigned char *command, *payload, response[0x5c];
  int l, thumbnail_length;
  unsigned char *thumbnail;

  payload = canon_fill_payload(0x10, 0, 0x50 << 8, 1, 1);
  command = canon_fill_command(0x60, 0x202, 0x17, 0x12, payload, 0x10);
  
  l = camera_control_write(camera_handle, 0x04, 0x10, 0x0, command, 0x60);
  if(l != 0x60)
    return -1;
  l = camera_bulk_read(camera_handle, response, 0x40);
  if(l != 0x40)
    return -1;

  thumbnail_length = (response[6] | (response[7] << 8) |
		      (response[8] << 16) | (response[9] << 24));
  fprintf(stderr, "thumbnail size %d/0x%x\n", thumbnail_length,
	  thumbnail_length);

  thumbnail = (unsigned char *)calloc(thumbnail_length, 1);
  l = camera_bulk_read(camera_handle, thumbnail, thumbnail_length);
  if(l != thumbnail_length)
    return -1;
  free(command);
  free(payload);
  free(thumbnail);
  return 0;
}

int canon_command_download_captured_image(usb_dev_handle *camera_handle)
{
  unsigned char *command, *payload, response[0x5c];
  int l, image_length, read_count, leftover;
  unsigned char *image;
  
  payload = canon_fill_payload(0x10, 0, 0x50 << 8, 2, 1);
  command = canon_fill_command(0x60, 0x202, 0x17, 0x12, payload, 0x10);
  
  l = camera_control_write(camera_handle, 0x04, 0x10, 0x0, command, 0x60);
  if(l != 0x60)
    return -1;
  l = camera_bulk_read(camera_handle, response, 0x40);
  if(l != 0x40)
    return -1;

  image_length = (response[6] | (response[7] << 8) |
		  (response[8] << 16) | (response[9] << 24));
  fprintf(stderr, "image size %d/0x%x\n", image_length, image_length);

  image = (unsigned char *)calloc(image_length, 1);

  read_count = 0;
  do {
    leftover = image_length - read_count;
    if(leftover > 0x5000)
      leftover = 0x5000;
    
    l = camera_bulk_read(camera_handle, image, leftover);
    if(l != leftover)
      return -1;
    read_count += leftover;
  } while(read_count < image_length);
  free(command);
  free(payload);
  free(image);
  return 0;
}

int canon_command_rcc_exit(usb_dev_handle *camera_handle)
{
  unsigned char *command, *payload, response[0x5c];
  int l;
  
  payload = canon_fill_payload(0x08, 1, 0, 0, 0);
  command = canon_fill_command(0x58, 0x201, 0x13, 0x12, payload, 0x8);
  
  l = camera_control_write(camera_handle, 0x04, 0x10, 0x0, command, 0x58);
  if(l != 0x58)
    return -1;
  l = camera_bulk_read(camera_handle, response, 0x40);
  if(l != 0x40)
    return -1;
  l = camera_bulk_read(camera_handle, response + 0x40, 0x1c);
  if(l != 0x1c)
    return -1;
  free(command);
  free(payload);
  return 0;
}

void close_camera(usb_dev_handle *camera_handle)
{
  /* close connection to the camera */
  fprintf(stderr, "Closing connection to camera... ");
  usb_release_interface(camera_handle, 0);
  usb_close(camera_handle);
  fprintf(stderr, "done.\n");
}

int main(int argc, char **argv)
{
  struct usb_device *camera;
  usb_dev_handle *camera_handle;
  int err, i;

  /* intialize USB library */
  usb_init();
  usb_find_busses();
  usb_find_devices();
  print_usb_info();

  /* search the USB bus for the appropriate camera */
  fprintf(stderr, "Searching for Canon Powershot S40 camera... ");
  camera = find_usb_device(0x04a9, 0x3056);
  if(camera == NULL) {
    fprintf(stderr, "not found.\n");
    exit(1);
  }
  else
    fprintf(stderr, "found.\n");

  /* claim the camera on the USB bus */
  fprintf(stderr, "Opening connection to camera... ");
  camera_handle = usb_open(camera);
  if(camera_handle == NULL) {
    fprintf(stderr, "failed.\n");
    exit(1);
  }
  else
    fprintf(stderr, "done.\n");
  if(usb_claim_interface(camera_handle, 0) < 0) {
    fprintf(stderr, "Error: could not claim USB interface to camera.\n");
    exit(1);
  }
  if(usb_set_configuration(camera_handle, 1) < 0) {
    fprintf(stderr, "Error: could not set configuration.\n");
    exit(1);
  }
  if(usb_set_altinterface(camera_handle, 0) < 0) {
    fprintf(stderr, "Error: could not set altinterface.\n");
    exit(1);
  }

  fprintf(stderr, "Initializing camera... ");
  if(canon_initialize_camera(camera_handle) < 0) {
    fprintf(stderr, "failed.\n");
    close_camera(camera_handle);
    exit(1);
  }
  else
    fprintf(stderr, "done.\n");

  fprintf(stderr, "Identifying camera... ");
  for(i = 0; i < 4; i++) {
    err = canon_command_identify_camera(camera_handle);
    if(err == 0)
      break;
  }
  if(err == 0)
    fprintf(stderr, "done.\n");
  else
    fprintf(stderr, "failed.\n");

  if(canon_command_get_picture_abilities(camera_handle) < 0)
    fprintf(stderr, "get camera abilites failed.\n");
  if(canon_command_lock_keys(camera_handle) < 0)
    fprintf(stderr, "lock keys failed.\n");
  if(canon_command_get_power_status(camera_handle) < 0)
    fprintf(stderr, "get power status failed.\n");
  sleep(1);

  
  if(canon_command_rcc_init(camera_handle) < 0)
    fprintf(stderr, "rcc init failed.\n");
  sleep(1);
  if(canon_command_rcc_unknown(camera_handle) < 0)
    fprintf(stderr, "rcc unknown failed.\n");
  if(canon_command_rcc_set_transfer_mode(camera_handle) < 0)
    fprintf(stderr, "rcc set transfer mode failed.\n");
  
  if(canon_command_rcc_release_shutter(camera_handle) < 0)
    fprintf(stderr, "rcc release shutter failed.\n");
  sleep(1);
  if(canon_command_download_captured_thumbnail(camera_handle) < 0)
    fprintf(stderr, "download thumbnail failed.\n");
  if(canon_command_download_captured_image(camera_handle) < 0)
    fprintf(stderr, "download image failed.\n");

  if(canon_command_rcc_exit(camera_handle) < 0)
    fprintf(stderr, "rcc exit failed.\n");
  

  close_camera(camera_handle);
  return 0;
}
