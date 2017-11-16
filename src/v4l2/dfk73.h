#ifndef TCAM_DFK73_H
#define TCAM_DFK73_H

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


int dfk73_prepare (int bus, int devnum);
void dfk73_prepare_all (void);
int dfk73_v4l2_prepare (char* devfile);
int dfk73_v4l2_set_framerate_index (int fd, int index);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TCAM_DFK73_H */
