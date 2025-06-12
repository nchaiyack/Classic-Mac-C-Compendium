/*
 * interface file to prep file reader writer
 * by Aaron Wohl (aw0g+@andrew.cmu.edu) jul 1990
 * Carnegie-Mellon University
 * Special Projects
 * Pittsburgh, PA 15213-3890
 * (412)-268-5032
 */

#define PREP_NAME "\pSoftKiss Prep"
#define PREP_RES_NUM (1234)

void sfk_init_state(sfk_state_pt s);
void sfk_invent_state(sfk_state_pt s);

struct sound_res_state_R {
	short prev_res;
	short sound_res;
};
typedef struct sound_res_state_R sound_res_state,*sound_res_state_pt;

sound_res_state sfk_use_sound_res(void);
void sfk_close_sound_res(sound_res_state sstate);

int sfk_read_prep(sfk_state_pt s);
int sfk_write_prep(sfk_state_pt s);
int sfk_dont_like_prep(sfk_state_pt s);

/*
 * note set_drvr_state returns the new updated state
 */
int sfk_get_drvr_state(int refnum,sfk_state_pt s);
int sfk_set_drvr_state(int refnum,sfk_state_pt s,int iokind);

