#ifndef POMDP_ROADMAP_H
#define POMDP_ROADMAP_H

carmen_list_t *carmen_roadmap_pomdp_generate_path(carmen_traj_point_t *robot,
						  carmen_roadmap_t *roadmap,
						  carmen_roadmap_t *
						  roadmap_without_people);
#endif
