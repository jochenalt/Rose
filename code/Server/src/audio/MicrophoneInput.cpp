/*
 * MicrophoneInput.cpp
 *
 *  Created on: Mar 22, 2018
 *      Author: jochenalt
 */

#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <sys/time.h>

#include <alsa/asoundlib.h>

#include <basics/util.h>
#include <audio/MicrophoneInput.h>


#define _(msgid) gettext (msgid)
static snd_pcm_sframes_t (*readi_func)(snd_pcm_t *handle, void *buffer, snd_pcm_uframes_t size);

static snd_pcm_t *handle = NULL;
static snd_pcm_format_t format;
static unsigned int channels;
static unsigned int rate;
size_t bits_per_sample = 0;
size_t bits_per_frame = 0;

#define error(...) do {\
	fprintf(stderr, "%s: %d: ", __FUNCTION__, __LINE__); \
	fprintf(stderr, __VA_ARGS__); \
	putc('\n', stderr); \
} while (0)

/* I/O error handler */
static void xrun(void)
{
	snd_pcm_status_t *status;
	int res;

	snd_pcm_status_alloca(&status);
	if ((res = snd_pcm_status(handle, status))<0) {
		error(_("status error: %s"), snd_strerror(res));
		exit(EXIT_FAILURE);
	}
	if (snd_pcm_status_get_state(status) == SND_PCM_STATE_XRUN) {
		struct timeval now, diff, tstamp;
		gettimeofday(&now, 0);
		snd_pcm_status_get_trigger_tstamp(status, &tstamp);
		timersub(&now, &tstamp, &diff);
		fprintf(stderr, _("%s!!! (at least %.3f ms long)\n"),
			"overrun",
			diff.tv_sec * 1000 + diff.tv_usec / 1000.0);

		if ((res = snd_pcm_prepare(handle))<0) {
			error(_("xrun: prepare error: %s"), snd_strerror(res));
			exit(EXIT_FAILURE);
		}
		return;		/* ok, data should be accepted again */
	} if (snd_pcm_status_get_state(status) == SND_PCM_STATE_DRAINING) {
			fprintf(stderr, _("capture stream format change? attempting recover...\n"));
			if ((res = snd_pcm_prepare(handle))<0) {
				error(_("xrun(DRAINING): prepare error: %s"), snd_strerror(res));
				exit(EXIT_FAILURE);
			}
			return;
	}
	error(_("read/write error, state = %s"), snd_pcm_state_name(snd_pcm_status_get_state(status)));
	exit(EXIT_FAILURE);
}

/* I/O suspend handler */
static void suspend(void)
{
	int res;

	fprintf(stderr, _("Suspended. Trying resume. ")); fflush(stderr);
	while ((res = snd_pcm_resume(handle)) == -EAGAIN)
		sleep(1);	/* wait until suspend flag is released */
	if (res < 0) {
		fprintf(stderr, _("Failed. Restarting stream. ")); fflush(stderr);
		if ((res = snd_pcm_prepare(handle)) < 0) {
			error(_("suspend: prepare error: %s"), snd_strerror(res));
			exit(EXIT_FAILURE);
		}
	}
	fprintf(stderr, _("Done.\n"));
}


ssize_t pcm_read(u_char *data, size_t rcount)
{
	ssize_t r;
	size_t result = 0;
	size_t count = rcount;

	while (count > 0) {
		r = readi_func(handle, data, count);
		if (r == -EAGAIN || (r >= 0 && (size_t)r < count)) {
			snd_pcm_wait(handle, 1000);
		} else if (r == -EPIPE) {
			xrun();
		} else if (r == -ESTRPIPE) {
			suspend();
		} else if (r < 0) {
			error(_("read error: %s"), snd_strerror(r));
			exit(EXIT_FAILURE);
		}
		if (r > 0) {
			result += r;
			count -= r;
			data += r * bits_per_frame / 8;
		}
	}
	return rcount;
}


MicrophoneInput::MicrophoneInput() {
}

MicrophoneInput::~MicrophoneInput() {
	if (handle) {
		snd_pcm_close(handle);
		handle = NULL;
	}
}

void MicrophoneInput::setup(int samplerate) {
    format = SND_PCM_FORMAT_S16_LE;
    rate = samplerate;
    channels = 1;

	const char *pcm_name = "default";
	snd_output_t *log;
	int err = snd_output_stdio_attach(&log, stderr, 0);
	assert(err >= 0);
	err = snd_pcm_open(&handle, pcm_name, SND_PCM_STREAM_CAPTURE, 0);
	assert(err >= 0);

	const bool nonblock = true;
	if (nonblock) {
		err = snd_pcm_nonblock(handle, 1);
		assert(err >= 0);
	}

	readi_func = snd_pcm_readi;

	snd_pcm_hw_params_t *params;
	snd_pcm_sw_params_t *swparams;
	snd_pcm_hw_params_alloca(&params);
	snd_pcm_sw_params_alloca(&swparams);
	err = snd_pcm_hw_params_any(handle, params);
	assert(err >= 0);
	err = snd_pcm_hw_params_set_access(handle, params,SND_PCM_ACCESS_RW_INTERLEAVED);
	assert(err >= 0);
	err = snd_pcm_hw_params_set_format(handle, params, format);
	assert(err >= 0);
	err = snd_pcm_hw_params_set_channels(handle, params, channels);
	assert(err >= 0);
	err = snd_pcm_hw_params_set_rate_near(handle, params, &rate, 0);
	assert(err >= 0);
	err = snd_pcm_hw_params(handle, params);
	assert(err >= 0);
	snd_pcm_sw_params_current(handle, swparams);
	assert(err >= 0);
	bits_per_sample = snd_pcm_format_physical_width(format);
	bits_per_frame = bits_per_sample * channels;

	cout << "using alsa default device for audio microphone input with "
		 << MicrophoneSampleRate << "Hz and latency of "
		 << (int)(getMicrophoneLatency()*1000.0) << "ms" << endl;
}

int MicrophoneInput::readMicrophoneInput(double frameBuffer[], unsigned frameBufferSize) {
	int byteBufferSize = frameBufferSize*2;
	uint8_t pcmBuffer[byteBufferSize*2];
	if (pcm_read(pcmBuffer, frameBufferSize) != frameBufferSize)
		return false;
	int bits = bits_per_frame;
	for (unsigned i = 0;i<frameBufferSize;i++) {
	    	int inputSample = (pcmBuffer[i*2+1] << 8) + (pcmBuffer[i*2]);
	       if (inputSample > (1<<(bits-1)))
	      	  	inputSample -= (1<<bits);
	      	frameBuffer[i] = (float)inputSample/(float)(1<<15);
	}
	return true;
}

double MicrophoneInput::getMicrophoneLatency() {
	return 0.25;
};
