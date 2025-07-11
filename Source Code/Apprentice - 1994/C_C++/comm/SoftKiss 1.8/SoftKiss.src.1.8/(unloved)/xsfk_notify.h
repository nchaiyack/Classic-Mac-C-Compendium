/*
 * SoftKiss notification interface
 * used internaly by device driver
 * by Aaron Wohl / N3LIW (aw0g+@andrew.cmu.edu) jul 1990
 * 6393 Penn Ave #303
 * Pittsburgh PA, 15206
 * work: (412)-268-5032
 * home: (412)-731-6159
 */

#pragma once

#define sfk_NOTE_note_mail  (0)
#define sfk_NOTE_note_error (1)
#define sfk_NOTE_note_kinds (2)


struct sfk_note_vars_R {
	char **snd_handle;		/*handle to mail sounds*/
	char **sicn_handle[sfk_NOTE_note_kinds];		/*handle to mail/error sicn*/
 	NMRec post_note_rec;	/*for posting dialogs/sound*/
 	NMRec blink_note_rec;	/*for blinking*/
};
typedef struct sfk_note_vars_R sfk_note_vars,*sfk_note_vars_pt;

void sfk_note_new_error(void);
void sfk_note_no_error(void);
void sfk_note_new_mail(void);
void sfk_note_old_mail(void);
void sfk_note_no_mail(void);
void sfk_note_uninit(void);
void sfk_note_init(void);
void set_event(int evnum,void (*dowhat)(),long dowhen);
int sfk_note_sound_init(int new_sound_res_num);
void sfk_note_done(void);

