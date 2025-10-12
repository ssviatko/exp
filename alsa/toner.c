#include <stdio.h>
#include <string.h>
#include <math.h>

#include <alsa/asoundlib.h>

#define SAMPLE_RATE 48000
#define KP_PULSE_LEN 5280 // 110ms
#define PULSE_LEN 2640 // 55ms

struct {
	int t1;
	int t2;
} tone_tbl[16] = {
	{ 1300, 1500 },
	{ 700, 900 },
	{ 700, 1100 },
	{ 900, 1100 },
	{ 700, 1300 },
	{ 900, 1300 },
	{ 1100, 1300 },
	{ 700, 1500 },
	{ 900, 1500 },
	{ 1100, 1500 },
	{ 350, 440 },
	{ 350, 440 },
	{ 350, 440 },
	{ 350, 440 },
	{ 350, 440 },
	{ 0, 2600 }
};

short *sine_wave(short *buffer, size_t sample_count, int freq)
{
	for (int i = 0; i < sample_count; i++) {
		buffer[i] = 10000 * sinf(2 * M_PI * freq * ((float)i / SAMPLE_RATE));
	}
	return buffer;
}

short *mf(short *buffer, size_t sample_count, int freq1, int freq2)
{
	short s1, s2;
	for (int i = 0; i < sample_count; i++) {
		s1 = 10000 * sinf(2 * M_PI * freq1 * ((float)i / SAMPLE_RATE));
		s2 = 10000 * sinf(2 * M_PI * freq2 * ((float)i / SAMPLE_RATE));
		buffer[i] = (s1 + s2) / 2;
	}
	return buffer;
}

short *silence(short *buffer, size_t sample_count)
{
	for (int i = 0; i < sample_count; ++i) {
		buffer[i] = 0;
	}
	return buffer;
}

void usage()
{
	printf("usage: toner <digits>\n");
	exit(EXIT_SUCCESS);
}

int main(int argc, char **argv)
{
	if (argc != 2) usage();
	printf("toner: %s\n", argv[1]);

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

	short samples[48000] = {0};

	snd_pcm_writei(pcm, mf(samples, KP_PULSE_LEN, 1100, 1700), KP_PULSE_LEN); // KP
	snd_pcm_writei(pcm, silence(samples, KP_PULSE_LEN), KP_PULSE_LEN); 

//	snd_pcm_writei(pcm, mf(samples, PULSE_LEN, 900, 1500), PULSE_LEN); // 8
//	snd_pcm_writei(pcm, silence(samples, PULSE_LEN), PULSE_LEN); 
//	snd_pcm_writei(pcm, mf(samples, PULSE_LEN, 700, 1300), PULSE_LEN); // 4
//	snd_pcm_writei(pcm, silence(samples, PULSE_LEN), PULSE_LEN); 
//	snd_pcm_writei(pcm, mf(samples, PULSE_LEN, 700, 1500), PULSE_LEN); // 7
//	snd_pcm_writei(pcm, silence(samples, PULSE_LEN), PULSE_LEN); 
//	snd_pcm_writei(pcm, mf(samples, PULSE_LEN, 900, 1500), PULSE_LEN); // 8
//	snd_pcm_writei(pcm, silence(samples, PULSE_LEN), PULSE_LEN); 
//	snd_pcm_writei(pcm, mf(samples, PULSE_LEN, 700, 1100), PULSE_LEN); // 2
//	snd_pcm_writei(pcm, silence(samples, PULSE_LEN), PULSE_LEN); 
//	snd_pcm_writei(pcm, mf(samples, PULSE_LEN, 900, 1300), PULSE_LEN); // 5
//	snd_pcm_writei(pcm, silence(samples, PULSE_LEN), PULSE_LEN); 
//	snd_pcm_writei(pcm, mf(samples, PULSE_LEN, 1300, 1500), PULSE_LEN); // 0
//	snd_pcm_writei(pcm, silence(samples, PULSE_LEN), PULSE_LEN); 
//	snd_pcm_writei(pcm, mf(samples, PULSE_LEN, 1300, 1500), PULSE_LEN); // 0
//	snd_pcm_writei(pcm, silence(samples, PULSE_LEN), PULSE_LEN); 
//	snd_pcm_writei(pcm, mf(samples, PULSE_LEN, 900, 1300), PULSE_LEN); // 5
//	snd_pcm_writei(pcm, silence(samples, PULSE_LEN), PULSE_LEN); 
//	snd_pcm_writei(pcm, mf(samples, PULSE_LEN, 900, 1100), PULSE_LEN); // 3
//	snd_pcm_writei(pcm, silence(samples, PULSE_LEN), PULSE_LEN); 

	for (int i = 0; i < strlen(argv[1]); ++i) {
		char digit = argv[1][i] & 0x0f;
		printf("playing digit %d: %d/%d\n", digit, tone_tbl[digit].t1, tone_tbl[digit].t2);
		snd_pcm_writei(pcm, mf(samples, PULSE_LEN, tone_tbl[digit].t1, tone_tbl[digit].t2), PULSE_LEN);
		snd_pcm_writei(pcm, silence(samples, PULSE_LEN), PULSE_LEN);
	}
	snd_pcm_writei(pcm, mf(samples, PULSE_LEN, 1500, 1700), PULSE_LEN); // ST
	snd_pcm_writei(pcm, silence(samples, PULSE_LEN), PULSE_LEN); 
	snd_pcm_drain(pcm);
	snd_pcm_close(pcm);

	return 0;
}

