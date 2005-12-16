char *robd_default_setup_string = "
[sonar]
firing_delay = 0
sonar_sets = sonset0 sonset1 sonset2 sonset3 sonset4 sonset5

[sonset0]
main_lobe =  209.43
blind_lobe = 1.74
side_lobe = 174.53
blind_lobe_attenuation = 2
side_lobe_attenuation = 1.2
range = 10000
sonars = son0 son1 son2 son3 son4 son5 son6 son7
son0 = CreateSonar son0 238.006 182.628 0.793353 0.608761 base
son1 = CreateSonar son1 182.628 238.006 0.608761 0.793353 base
son2 = CreateSonar son2 114.805 277.164 0.382683 0.92388 base
son3 = CreateSonar son3 39.1579 297.433 0.130526 0.991445 base
son4 = CreateSonar son4 -39.1579 297.433 -0.130526 0.991445 base
son5 = CreateSonar son5 -114.805 277.164 -0.382683 0.92388 base
son6 = CreateSonar son6 -182.628 238.006 -0.608761 0.793353 base
son7 = CreateSonar son7 -238.006 182.628 -0.793353 0.608761 base

[sonset1]
main_lobe =  209.43
blind_lobe = 1.74
side_lobe = 174.53
blind_lobe_attenuation = 2
side_lobe_attenuation = 1.2
range = 10000
sonars = son0 son1 son2 son3 son4 son5 son6 son7
son0 = CreateSonar son0 238.006 182.628 0.793353 0.608761 base
son1 = CreateSonar son1 182.628 238.006 0.608761 0.793353 base
son2 = CreateSonar son2 114.805 277.164 0.382683 0.92388 base
son3 = CreateSonar son3 39.1579 297.433 0.130526 0.991445 base
son4 = CreateSonar son4 -39.1579 297.433 -0.130526 0.991445 base
son5 = CreateSonar son5 -114.805 277.164 -0.382683 0.92388 base
son6 = CreateSonar son6 -182.628 238.006 -0.608761 0.793353 base
son7 = CreateSonar son7 -238.006 182.628 -0.793353 0.608761 base

[sonset2]
main_lobe =  209.43
blind_lobe = 1.74
side_lobe = 174.53
blind_lobe_attenuation = 2
side_lobe_attenuation = 1.2
range = 10000
sonars = son0 son1 son2 son3 son4 son5 son6 son7
son0 = CreateSonar son0 -277.164 114.805 -0.92388 0.382683 base
son1 = CreateSonar son1 -297.433 39.1579 -0.991445 0.130526 base
son2 = CreateSonar son2 -297.433 -39.1579 -0.991445 -0.130526 base
son3 = CreateSonar son3 -277.164 -114.805 -0.92388 -0.382683 base
son4 = CreateSonar son4 -238.006 -182.628 -0.793353 -0.608761 base
son5 = CreateSonar son5 -182.628 -238.006 -0.608761 -0.793353 base
son6 = CreateSonar son6 -114.805 -277.164 -0.382683 -0.92388 base
son7 = CreateSonar son7 -39.1579 -297.433 -0.130526 -0.991445 base

[sonset3]
main_lobe =  209.43
blind_lobe = 1.74
side_lobe = 174.53
blind_lobe_attenuation = 2
side_lobe_attenuation = 1.2
range = 10000
sonars = son0 son1 son2 son3 son4 son5 son6 son7
son0 = CreateSonar son0 -277.164 114.805 -0.92388 0.382683 base
son1 = CreateSonar son1 -297.433 39.1579 -0.991445 0.130526 base
son2 = CreateSonar son2 -297.433 -39.1579 -0.991445 -0.130526 base
son3 = CreateSonar son3 -277.164 -114.805 -0.92388 -0.382683 base
son4 = CreateSonar son4 -238.006 -182.628 -0.793353 -0.608761 base
son5 = CreateSonar son5 -182.628 -238.006 -0.608761 -0.793353 base
son6 = CreateSonar son6 -114.805 -277.164 -0.382683 -0.92388 base
son7 = CreateSonar son7 -39.1579 -297.433 -0.130526 -0.991445 base

[sonset4]
main_lobe =  209.43
blind_lobe = 1.74
side_lobe = 174.53
blind_lobe_attenuation = 2
side_lobe_attenuation = 1.2
range = 10000
sonars = son0 son1 son2 son3 son4 son5 son6 son7
son0 = CreateSonar son0 39.1579 -297.433 0.130526 -0.991445 base
son1 = CreateSonar son1 114.805 -277.164 0.382683 -0.92388 base
son2 = CreateSonar son2 182.628 -238.006 0.608761 -0.793353 base
son3 = CreateSonar son3 238.006 -182.628 0.793353 -0.608761 base
son4 = CreateSonar son4 277.164 -114.805 0.92388 -0.382683 base
son5 = CreateSonar son5 297.433 -39.1579 0.991445 -0.130526 base
son6 = CreateSonar son6 297.433 39.1579 0.991445 0.130526 base
son7 = CreateSonar son7 277.164 114.805 0.92388 0.382683 base

[sonset5]
main_lobe =  209.43
blind_lobe = 1.74
side_lobe = 174.53
blind_lobe_attenuation = 2
side_lobe_attenuation = 1.2
range = 10000
sonars = son0 son1 son2 son3 son4 son5 son6 son7
son0 = CreateSonar son0 39.1579 -297.433 0.130526 -0.991445 base
son1 = CreateSonar son1 114.805 -277.164 0.382683 -0.92388 base
son2 = CreateSonar son2 182.628 -238.006 0.608761 -0.793353 base
son3 = CreateSonar son3 238.006 -182.628 0.793353 -0.608761 base
son4 = CreateSonar son4 277.164 -114.805 0.92388 -0.382683 base
son5 = CreateSonar son5 297.433 -39.1579 0.991445 -0.130526 base
son6 = CreateSonar son6 297.433 39.1579 0.991445 0.130526 base
son7 = CreateSonar son7 277.164 114.805 0.92388 0.382683 base

[infrared]
infrared_sets = irset0 irset1 irset2 irset3 irset4 irset5

[irset0]
main_lobe = 261.79
range = 350
infrareds = ir0 ir1 ir2 ir3 ir4 ir5 ir6 ir7
ir0 = CreateInfrared ir0 238.006 182.628 0.793353 0.608761 base
ir1 = CreateInfrared ir1 182.628 238.006 0.608761 0.793353 base
ir2 = CreateInfrared ir2 114.805 277.164 0.382683 0.92388 base
ir3 = CreateInfrared ir3 39.1579 297.433 0.130526 0.991445 base
ir4 = CreateInfrared ir4 -39.1579 297.433 -0.130526 0.991445 base
ir5 = CreateInfrared ir5 -114.805 277.164 -0.382683 0.92388 base
ir6 = CreateInfrared ir6 -182.628 238.006 -0.608761 0.793353 base
ir7 = CreateInfrared ir7 -238.006 182.628 -0.793353 0.608761 base

[irset1]
main_lobe = 261.79
range = 350
infrareds = ir0 ir1 ir2 ir3 ir4 ir5 ir6 ir7
ir0 = CreateInfrared ir0 238.006 182.628 0.793353 0.608761 base
ir1 = CreateInfrared ir1 182.628 238.006 0.608761 0.793353 base
ir2 = CreateInfrared ir2 114.805 277.164 0.382683 0.92388 base
ir3 = CreateInfrared ir3 39.1579 297.433 0.130526 0.991445 base
ir4 = CreateInfrared ir4 -39.1579 297.433 -0.130526 0.991445 base
ir5 = CreateInfrared ir5 -114.805 277.164 -0.382683 0.92388 base
ir6 = CreateInfrared ir6 -182.628 238.006 -0.608761 0.793353 base
ir7 = CreateInfrared ir7 -238.006 182.628 -0.793353 0.608761 base

[irset2]
id = irset1t
main_lobe = 261.79
range = 350
infrareds = ir0 ir1 ir2 ir3 ir4 ir5 ir6 ir7
ir0 = CreateInfrared ir0 -277.164 114.805 -0.92388 0.382683 base
ir1 = CreateInfrared ir1 -297.433 39.1579 -0.991445 0.130526 base
ir2 = CreateInfrared ir2 -297.433 -39.1579 -0.991445 -0.130526 base
ir3 = CreateInfrared ir3 -277.164 -114.805 -0.92388 -0.382683 base
ir4 = CreateInfrared ir4 -238.006 -182.628 -0.793353 -0.608761 base
ir5 = CreateInfrared ir5 -182.628 -238.006 -0.608761 -0.793353 base
ir6 = CreateInfrared ir6 -114.805 -277.164 -0.382683 -0.92388 base
ir7 = CreateInfrared ir7 -39.1579 -297.433 -0.130526 -0.991445 base

[irset3]
main_lobe = 261.79
range = 350
infrareds = ir0 ir1 ir2 ir3 ir4 ir5 ir6 ir7
ir0 = CreateInfrared ir0 -277.164 114.805 -0.92388 0.382683 base
ir1 = CreateInfrared ir1 -297.433 39.1579 -0.991445 0.130526 base
ir2 = CreateInfrared ir2 -297.433 -39.1579 -0.991445 -0.130526 base
ir3 = CreateInfrared ir3 -277.164 -114.805 -0.92388 -0.382683 base
ir4 = CreateInfrared ir4 -238.006 -182.628 -0.793353 -0.608761 base
ir5 = CreateInfrared ir5 -182.628 -238.006 -0.608761 -0.793353 base
ir6 = CreateInfrared ir6 -114.805 -277.164 -0.382683 -0.92388 base
ir7 = CreateInfrared ir7 -39.1579 -297.433 -0.130526 -0.991445 base

[irset4]
main_lobe = 261.79
range = 350
infrareds = ir0 ir1 ir2 ir3 ir4 ir5 ir6 ir7
ir0 = CreateInfrared ir0 39.1579 -297.433 0.130526 -0.991445 base
ir1 = CreateInfrared ir1 114.805 -277.164 0.382683 -0.92388 base
ir2 = CreateInfrared ir2 182.628 -238.006 0.608761 -0.793353 base
ir3 = CreateInfrared ir3 238.006 -182.628 0.793353 -0.608761 base
ir4 = CreateInfrared ir4 277.164 -114.805 0.92388 -0.382683 base
ir5 = CreateInfrared ir5 297.433 -39.1579 0.991445 -0.130526 base
ir6 = CreateInfrared ir6 297.433 39.1579 0.991445 0.130526 base
ir7 = CreateInfrared ir7 277.164 114.805 0.92388 0.382683 base

[irset5]
main_lobe = 261.79
range = 350
infrareds = ir0 ir1 ir2 ir3 ir4 ir5 ir6 ir7
ir0 = CreateInfrared ir0 39.1579 -297.433 0.130526 -0.991445 base
ir1 = CreateInfrared ir1 114.805 -277.164 0.382683 -0.92388 base
ir2 = CreateInfrared ir2 182.628 -238.006 0.608761 -0.793353 base
ir3 = CreateInfrared ir3 238.006 -182.628 0.793353 -0.608761 base
ir4 = CreateInfrared ir4 277.164 -114.805 0.92388 -0.382683 base
ir5 = CreateInfrared ir5 297.433 -39.1579 0.991445 -0.130526 base
ir6 = CreateInfrared ir6 297.433 39.1579 0.991445 0.130526 base
ir7 = CreateInfrared ir7 277.164 114.805 0.92388 0.382683 base

[bumper]
bumper_sets = bumperset0 bumperset1 bumperset2 bumperset3 bumperset4 bumperset5

[bumperset0]
bumpers = bumperarc bumper8 bumper9 bumper10 bumper11
bumperarc = CreateBumperArc 8 bumper 0 0 0.6545 2.0944 base
bumper8 = CreateBumper bumper8 0 0 0.5 0.866 base
bumper9 = CreateBumper bumper9 0 0 0.5 0.866 base
bumper10 = CreateBumper bumper10 0 0 -0.5 0.866 base
bumper11 = CreateBumper bumper11 0 0 -0.5 0.866 base

[bumperset1]
bumpers = bumperarc
bumperarc = CreateBumperArc 8 bumper 0 0 0.6545 2.0944 base

[bumperset2]
bumpers = bumperarc bumper8 bumper9 bumper10 bumper11
bumperarc = CreateBumperArc 8 bumper 0 0 2.749 2.0944 base
bumper8 = CreateBumper bumper8 0 0 -1.0 0.0 base
bumper9 = CreateBumper bumper9 0 0 -1.0 0.0 base
bumper10 = CreateBumper bumper10 0 0 -0.5 -0.866 base
bumper11 = CreateBumper bumper11 0 0 -0.5 -0.866 base

[bumperset3]
bumpers = bumperarc
bumperarc = CreateBumperArc 8 bumper 0 0 2.749 2.0944 base

[bumperset4]
bumpers = bumperarc bumper8 bumper9 bumper10 bumper11
bumperarc = CreateBumperArc 8 bumper 0 0 4.843 2.0944 base
bumper8 = CreateBumper bumper8 0 0 0.5 -0.866 base
bumper9 = CreateBumper bumper9 0 0 0.5 -0.866 base
bumper10 = CreateBumper bumper10 0 0 1.0 0.0 base
bumper11 = CreateBumper bumper11 0 0 1.0 0.0 base

[bumperset5]
bumpers = bumperarc
bumperarc = CreateBumperArc 8 bumper 0 0 4.843 2.0944 base";

