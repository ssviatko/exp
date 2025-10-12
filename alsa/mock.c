#include <stdio.h>
#include <string.h>
#include <math.h>

#include <alsa/asoundlib.h>

#define SAMPLE_RATE 48000

void playback(short *buffer)
{
	snd_pcm_t *pcm;
	snd_pcm_open(&pcm, "default", SND_PCM_STREAM_PLAYBACK, 0);

	snd_pcm_hw_params_t *hw_params;
	snd_pcm_hw_params_alloca(&hw_params);

	snd_pcm_hw_params_any(pcm, hw_params);
	snd_pcm_hw_params_set_access(pcm, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
	snd_pcm_hw_params_set_format(pcm, hw_params, SND_PCM_FORMAT_S16_LE);
	snd_pcm_hw_params_set_channels(pcm, hw_params, 1);
	snd_pcm_hw_params_set_rate(pcm, hw_params, SAMPLE_RATE, 0);
	snd_pcm_hw_params_set_periods(pcm, hw_params, 10, 0);
	snd_pcm_hw_params_set_period_time(pcm, hw_params, 100000, 0); // 0.1 seconds

	snd_pcm_hw_params(pcm, hw_params);

	printf("playing 5 second buffer...\n");

	snd_pcm_writei(pcm, buffer, SAMPLE_RATE * 5); 
	snd_pcm_drain(pcm);
	snd_pcm_close(pcm);
}

void record(short *buffer)
{
	snd_pcm_t *pcm;
	snd_pcm_open(&pcm, "default", SND_PCM_STREAM_CAPTURE, 0);

	snd_pcm_hw_params_t *hw_params;
	snd_pcm_hw_params_alloca(&hw_params);

	snd_pcm_hw_params_any(pcm, hw_params);
	snd_pcm_hw_params_set_access(pcm, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
	snd_pcm_hw_params_set_format(pcm, hw_params, SND_PCM_FORMAT_S16_LE);
	snd_pcm_hw_params_set_channels(pcm, hw_params, 1);
	snd_pcm_hw_params_set_rate(pcm, hw_params, SAMPLE_RATE, 0);
	snd_pcm_hw_params_set_periods(pcm, hw_params, 10, 0);
	snd_pcm_hw_params_set_period_time(pcm, hw_params, 100000, 0); // 0.1 seconds

	snd_pcm_hw_params(pcm, hw_params);
	
	printf("recording 5 second buffer...\n");

	short tmprd[(SAMPLE_RATE / 10)]; // up to .1 secs temp buffer + padding
				       //
	// loop for 5 seconds, or 50 period times
	int loops = 50;
	int buffptr = 0;
	int rc;
	while (loops-- > 0) {
		rc = snd_pcm_readi(pcm, tmprd, SAMPLE_RATE / 10);
    		if (rc == -EPIPE) {
      			/* EPIPE means overrun */
      			fprintf(stderr, "overrun occurred\n");
      			snd_pcm_prepare(pcm);
    		} else if (rc < 0) {
      			fprintf(stderr, "error from read: %s\n", snd_strerror(rc));
    		} else {
			printf("writing %d bytes at %d.\n", rc *2, buffptr);
			memcpy((char *)buffer + buffptr, tmprd, rc * 2);
			buffptr += rc * 2;
		}
	}
}

int main(int argc, char **argv)
{
	short buffer[SAMPLE_RATE * 5]; // enough for 6 seconds
	
	printf("buffer size: %ld bytes.\n", sizeof(buffer));	

	record(buffer);
	playback(buffer);
	return 0;
}

