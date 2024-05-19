/******************************************************************************/
#include "stdafx.h"
#if SUPPORT_SAMPLERATE // TODO: add support on all platforms
#include "../../../ThirdPartyLibs/SampleRate/lib/src/samplerate.h"
#include "../../../ThirdPartyLibs/begin.h"
#include "../../../ThirdPartyLibs/end.h"
#endif
namespace EE {
/******************************************************************************/
static Stereo StereoZero = {0, 0};
I16 SoundResampler::srcMono(Int pos) C {
    if (InRange(pos, src_samples))
        return src_mono[pos];
    pos += buffer.samples;
    if (InRange(pos, buffer.samples))
        return buffer.mono[pos];
    return 0;
}
C Stereo &SoundResampler::srcStereo(Int pos) C {
    if (InRange(pos, src_samples))
        return src_stereo[pos];
    pos += buffer.samples;
    if (InRange(pos, buffer.samples))
        return buffer.stereo[pos];
    return StereoZero;
}
/******************************************************************************/
INLINE void SoundResampler::process(void Process(I16 &sample, Flt value)) {
    const Int buffer_max = Elms(buffer.mono);
    const Int buffer_max1 = buffer_max - 1;
    if (speed == 1) // no resample needed
    {
        if (buffer.samples) // has anything in the buffer
        {
            Int copy_start = ((buffer.samples < buffer_max) ? 0                            // didn't finish filling up the buffer = use all its samples
                                                            : 1 + (buffer.offset > 0.5f)), // skip src_sample_posP previous sample, and if sample offset crossed half, then skip src_sample_pos0 sample too
                copy_samples = Min(buffer.samples - copy_start, dest_samples);
            dest_samples -= copy_samples;
            switch (src_channels) {
            case 1: // SRC MONO
            {
                C I16 *src_mono = buffer.mono + copy_start;
                switch (dest_channels) {
                case 1:
                    REP(copy_samples)
                    Process(*dest_mono++, *src_mono++ * volume[0]);
                    break; // DEST MONO
                case 2:
                    REP(copy_samples) // DEST STEREO
                    {
                        I16 sample = *src_mono++;
                        Process(dest_stereo->l, sample * volume[0]);
                        Process(dest_stereo->r, sample * volume[1]);
                        dest_stereo++;
                    }
                    break;
                }
                buffer.samples -= copy_samples + copy_start;
                MoveFastN(buffer.mono, src_mono, buffer.samples);
            } break;

            case 2: // SRC STEREO
            {
                C Stereo *src_stereo = buffer.stereo + copy_start;
                switch (dest_channels) {
                case 1:
                    REP(copy_samples) // DEST MONO
                    {
                        Process(*dest_mono++, src_stereo->l * volume[0] + src_stereo->r * volume[1]);
                        src_stereo++;
                    }
                    break;
                case 2:
                    REP(copy_samples) // DEST STEREO
                    {
                        Process(dest_stereo->l, src_stereo->l * volume[0]);
                        Process(dest_stereo->r, src_stereo->r * volume[1]);
                        src_stereo++;
                        dest_stereo++;
                    }
                    break;
                }
                buffer.samples -= copy_samples + copy_start;
                MoveFastN(buffer.stereo, src_stereo, buffer.samples);
            } break;
            }
        }
        Int samples = Min(src_samples, dest_samples);
        dest_samples -= samples;
        src_samples -= samples;
        switch (dest_channels) {
        case 1:
            switch (src_channels) // DEST MONO
            {
            case 1: // SRC MONO
#if 1
                REP(samples >> 2) {
                    Process(dest_mono[0], src_mono[0] * volume[0]);
                    Process(dest_mono[1], src_mono[1] * volume[0]);
                    Process(dest_mono[2], src_mono[2] * volume[0]);
                    Process(dest_mono[3], src_mono[3] * volume[0]);
                    src_mono += 4;
                    dest_mono += 4;
                }
                samples &= 3;
#endif
                REP(samples) {
                    Process(*dest_mono++, *src_mono++ * volume[0]);
                }
                break;

            case 2: // SRC STEREO
#if 1
                REP(samples >> 2) {
                    Process(dest_mono[0], src_stereo[0].l * volume[0] + src_stereo[0].r * volume[1]);
                    Process(dest_mono[1], src_stereo[1].l * volume[0] + src_stereo[1].r * volume[1]);
                    Process(dest_mono[2], src_stereo[2].l * volume[0] + src_stereo[2].r * volume[1]);
                    Process(dest_mono[3], src_stereo[3].l * volume[0] + src_stereo[3].r * volume[1]);
                    src_stereo += 4;
                    dest_mono += 4;
                }
                samples &= 3;
#endif
                REP(samples) {
                    Process(*dest_mono++, src_stereo->l * volume[0] + src_stereo->r * volume[1]);
                    src_stereo++;
                }
                break;
            }
            break;

        case 2:
            switch (src_channels) // DEST STEREO
            {
            case 1: // SRC MONO
#if 1
                REP(samples >> 2) {
                    I16 sample = src_mono[0];
                    Process(dest_stereo[0].l, sample * volume[0]);
                    Process(dest_stereo[0].r, sample * volume[1]);
                    sample = src_mono[1];
                    Process(dest_stereo[1].l, sample * volume[0]);
                    Process(dest_stereo[1].r, sample * volume[1]);
                    sample = src_mono[2];
                    Process(dest_stereo[2].l, sample * volume[0]);
                    Process(dest_stereo[2].r, sample * volume[1]);
                    sample = src_mono[3];
                    Process(dest_stereo[3].l, sample * volume[0]);
                    Process(dest_stereo[3].r, sample * volume[1]);
                    src_mono += 4;
                    dest_stereo += 4;
                }
                samples &= 3;
#endif
                REP(samples) {
                    I16 sample = *src_mono++;
                    Process(dest_stereo->l, sample * volume[0]);
                    Process(dest_stereo->r, sample * volume[1]);
                    dest_stereo++;
                }
                break;

            case 2: // SRC STEREO
#if 1
                REP(samples >> 2) {
                    Process(dest_stereo[0].l, src_stereo[0].l * volume[0]);
                    Process(dest_stereo[0].r, src_stereo[0].r * volume[1]);
                    Process(dest_stereo[1].l, src_stereo[1].l * volume[0]);
                    Process(dest_stereo[1].r, src_stereo[1].r * volume[1]);
                    Process(dest_stereo[2].l, src_stereo[2].l * volume[0]);
                    Process(dest_stereo[2].r, src_stereo[2].r * volume[1]);
                    Process(dest_stereo[3].l, src_stereo[3].l * volume[0]);
                    Process(dest_stereo[3].r, src_stereo[3].r * volume[1]);
                    src_stereo += 4;
                    dest_stereo += 4;
                }
                samples &= 3;
#endif
                REP(samples) {
                    Process(dest_stereo->l, src_stereo->l * volume[0]);
                    Process(dest_stereo->r, src_stereo->r * volume[1]);
                    src_stereo++;
                    dest_stereo++;
                }
                break;
            }
            break;
        }
    } else // resample
    {
        // read to buffer
        if (buffer.samples < buffer_max1) // at the start copy 1 less, to make src[src_sample_posP] always empty/zero, and first dest sample directly mapped to src[src_sample_pos0]
        {
            Int copy_samples = Min(src_samples, buffer_max1 - buffer.samples);
            Int copy_samples_channels = copy_samples * src_channels;
            CopyFastN(buffer.mono + buffer.samples * src_channels, src_mono, copy_samples_channels);
            buffer.samples += copy_samples;
            src_samples -= copy_samples;
            src_mono += copy_samples_channels;
        }

        Int dest_sample_pos = 0,
            src_sample_pos;
        for (;; dest_sample_pos++) {
            Flt src_sample_posf = dest_sample_pos * speed + buffer.offset; // add leftover offset from previous operations
            src_sample_pos = Trunc(src_sample_posf);
            Flt frac = src_sample_posf - src_sample_pos; // calculate sample position fraction
            if (src_sample_pos >= src_samples && !end || dest_sample_pos >= dest_samples) {
                buffer.offset = frac;
                break;
            } // if reached the end of any buffer then stop and remember current 'buffer.offset' for future operations !! this has to be done after calculating 'src_sample_pos' which is used later !!

            if (speed < 1) { // cubic
                Int src_sample_posP = src_sample_pos - 3;
                Int src_sample_pos0 = src_sample_pos - 2;
                Int src_sample_pos1 = src_sample_pos - 1;
                Int src_sample_pos2 = src_sample_pos;
                Vec4 w;
                Lerp4Weights(w, frac);
                if (src_sample_posP >= 0 && src_sample_pos2 < src_samples) // all samples in src range
                    switch (dest_channels) {
                    case 1:
                        switch (src_channels) // DEST MONO
                        {
                        case 1: // SRC MONO
                        {
                            Process(*dest_mono++, (src_mono[src_sample_posP] * w.x + src_mono[src_sample_pos0] * w.y + src_mono[src_sample_pos1] * w.z + src_mono[src_sample_pos2] * w.w) * volume[0]);
                        } break;

                        case 2: // SRC STEREO
                        {
                            C Stereo &sP = src_stereo[src_sample_posP],
                                     &s0 = src_stereo[src_sample_pos0],
                                     &s1 = src_stereo[src_sample_pos1],
                                     &s2 = src_stereo[src_sample_pos2];
                            Process(*dest_mono++, (sP.l * w.x + s0.l * w.y + s1.l * w.z + s2.l * w.w) * volume[0]         // Lerp(s0.l, s1.l, frac)
                                                      + (sP.r * w.x + s0.r * w.y + s1.r * w.z + s2.r * w.w) * volume[1]); // Lerp(s0.r, s1.r, frac)
                        } break;
                        }
                        break;

                    case 2:
                        switch (src_channels) // DEST STEREO
                        {
                        case 1: // SRC MONO
                        {
                            Flt sample = src_mono[src_sample_posP] * w.x + src_mono[src_sample_pos0] * w.y + src_mono[src_sample_pos1] * w.z + src_mono[src_sample_pos2] * w.w;
                            Process(dest_stereo->l, sample * volume[0]);
                            Process(dest_stereo->r, sample * volume[1]);
                            dest_stereo++;
                        } break;

                        case 2: // SRC STEREO
                        {
                            C Stereo &sP = src_stereo[src_sample_posP],
                                     &s0 = src_stereo[src_sample_pos0],
                                     &s1 = src_stereo[src_sample_pos1],
                                     &s2 = src_stereo[src_sample_pos2];
                            Process(dest_stereo->l, (sP.l * w.x + s0.l * w.y + s1.l * w.z + s2.l * w.w) * volume[0]); // Lerp(s0.l, s1.l, frac)
                            Process(dest_stereo->r, (sP.r * w.x + s0.r * w.y + s1.r * w.z + s2.r * w.w) * volume[1]); // Lerp(s0.r, s1.r, frac)
                            dest_stereo++;
                        } break;
                        }
                        break;
                    }
                else // some used from the buffer
                    switch (dest_channels) {
                    case 1:
                        switch (src_channels) // DEST MONO
                        {
                        case 1: // SRC MONO
                        {
                            Process(*dest_mono++, (srcMono(src_sample_posP) * w.x + srcMono(src_sample_pos0) * w.y + srcMono(src_sample_pos1) * w.z + srcMono(src_sample_pos2) * w.w) * volume[0]);
                        } break;

                        case 2: // SRC STEREO
                        {
                            C Stereo &sP = srcStereo(src_sample_posP),
                                     &s0 = srcStereo(src_sample_pos0),
                                     &s1 = srcStereo(src_sample_pos1),
                                     &s2 = srcStereo(src_sample_pos2);
                            Process(*dest_mono++, (sP.l * w.x + s0.l * w.y + s1.l * w.z + s2.l * w.w) * volume[0]         // Lerp(s0.l, s1.l, frac)
                                                      + (sP.r * w.x + s0.r * w.y + s1.r * w.z + s2.r * w.w) * volume[1]); // Lerp(s0.r, s1.r, frac)
                        } break;
                        }
                        break;

                    case 2:
                        switch (src_channels) // DEST STEREO
                        {
                        case 1: // SRC MONO
                        {
                            Flt sample = srcMono(src_sample_posP) * w.x + srcMono(src_sample_pos0) * w.y + srcMono(src_sample_pos1) * w.z + srcMono(src_sample_pos2) * w.w;
                            Process(dest_stereo->l, sample * volume[0]);
                            Process(dest_stereo->r, sample * volume[1]);
                            dest_stereo++;
                        } break;

                        case 2: // SRC STEREO
                        {
                            C Stereo &sP = srcStereo(src_sample_posP),
                                     &s0 = srcStereo(src_sample_pos0),
                                     &s1 = srcStereo(src_sample_pos1),
                                     &s2 = srcStereo(src_sample_pos2);
                            Process(dest_stereo->l, (sP.l * w.x + s0.l * w.y + s1.l * w.z + s2.l * w.w) * volume[0]); // Lerp(s0.l, s1.l, frac)
                            Process(dest_stereo->r, (sP.r * w.x + s0.r * w.y + s1.r * w.z + s2.r * w.w) * volume[1]); // Lerp(s0.r, s1.r, frac)
                            dest_stereo++;
                        } break;
                        }
                        break;
                    }
            } else // speed>1 TODO: for speed>2 we could blend multiple samples however in tests results were unnoticable
            {      // linear
                Int src_sample_pos0 = src_sample_pos - 2;
                Int src_sample_pos1 = src_sample_pos - 1;
                Flt frac1 = 1 - frac;
                if (src_sample_pos0 >= 0 && src_sample_pos1 < src_samples) // all samples in src range
                    switch (dest_channels) {
                    case 1:
                        switch (src_channels) // DEST MONO
                        {
                        case 1: // SRC MONO
                        {
                            Process(*dest_mono++, (src_mono[src_sample_pos0] * frac1 + src_mono[src_sample_pos1] * frac) * volume[0]); // Lerp(src_mono[src_sample_pos0], src_mono[src_sample_pos1], frac)
                        } break;

                        case 2: // SRC STEREO
                        {
                            C Stereo &s0 = src_stereo[src_sample_pos0],
                                     &s1 = src_stereo[src_sample_pos1];
                            Process(*dest_mono++, (s0.l * frac1 + s1.l * frac) * volume[0]         // Lerp(s0.l, s1.l, frac)
                                                      + (s0.r * frac1 + s1.r * frac) * volume[1]); // Lerp(s0.r, s1.r, frac)
                        } break;
                        }
                        break;

                    case 2:
                        switch (src_channels) // DEST STEREO
                        {
                        case 1: // SRC MONO
                        {
                            Flt sample = src_mono[src_sample_pos0] * frac1 + src_mono[src_sample_pos1] * frac; // Lerp(src_mono[src_sample_pos0], src_mono[src_sample_pos1], frac)
                            Process(dest_stereo->l, sample * volume[0]);
                            Process(dest_stereo->r, sample * volume[1]);
                            dest_stereo++;
                        } break;

                        case 2: // SRC STEREO
                        {
                            C Stereo &s0 = src_stereo[src_sample_pos0],
                                     &s1 = src_stereo[src_sample_pos1];
                            Process(dest_stereo->l, (s0.l * frac1 + s1.l * frac) * volume[0]); // Lerp(s0.l, s1.l, frac)
                            Process(dest_stereo->r, (s0.r * frac1 + s1.r * frac) * volume[1]); // Lerp(s0.r, s1.r, frac)
                            dest_stereo++;
                        } break;
                        }
                        break;
                    }
                else // some used from the buffer
                    switch (dest_channels) {
                    case 1:
                        switch (src_channels) // DEST MONO
                        {
                        case 1: // SRC MONO
                        {
                            Process(*dest_mono++, (srcMono(src_sample_pos0) * frac1 + srcMono(src_sample_pos1) * frac) * volume[0]); // Lerp(srcMono(src_sample_pos0], srcMono(src_sample_pos1], frac)
                        } break;

                        case 2: // SRC STEREO
                        {
                            C Stereo &s0 = srcStereo(src_sample_pos0),
                                     &s1 = srcStereo(src_sample_pos1);
                            Process(*dest_mono++, (s0.l * frac1 + s1.l * frac) * volume[0]         // Lerp(s0.l, s1.l, frac)
                                                      + (s0.r * frac1 + s1.r * frac) * volume[1]); // Lerp(s0.r, s1.r, frac)
                        } break;
                        }
                        break;

                    case 2:
                        switch (src_channels) // DEST STEREO
                        {
                        case 1: // SRC MONO
                        {
                            Flt sample = srcMono(src_sample_pos0) * frac1 + srcMono(src_sample_pos1) * frac; // Lerp(srcMono(src_sample_pos0], srcMono(src_sample_pos1], frac)
                            Process(dest_stereo->l, sample * volume[0]);
                            Process(dest_stereo->r, sample * volume[1]);
                            dest_stereo++;
                        } break;

                        case 2: // SRC STEREO
                        {
                            C Stereo &s0 = srcStereo(src_sample_pos0),
                                     &s1 = srcStereo(src_sample_pos1);
                            Process(dest_stereo->l, (s0.l * frac1 + s1.l * frac) * volume[0]); // Lerp(s0.l, s1.l, frac)
                            Process(dest_stereo->r, (s0.r * frac1 + s1.r * frac) * volume[1]); // Lerp(s0.r, s1.r, frac)
                            dest_stereo++;
                        } break;
                        }
                        break;
                    }
            }
        }
        dest_samples -= dest_sample_pos;
        src_samples -= src_sample_pos;
        src_mono += src_sample_pos * src_channels;

        // copy last samples into buffer
        Int samples = src_sample_pos; // read samples
        if (samples >= buffer_max)    // put all into buffer and discard old
        {
            buffer.samples = buffer_max;
            Int buffer_samples_channels = buffer.samples * src_channels;
            CopyFastN(buffer.mono, src_mono - buffer_samples_channels, buffer_samples_channels);
        } else {
            Int new_buffer_samples = buffer.samples + samples;
            if (new_buffer_samples > buffer_max) // remove some
            {
                Int remove = new_buffer_samples - buffer_max;
                buffer.samples -= remove;
                MoveFastN(buffer.mono, buffer.mono + remove * src_channels, buffer.samples * src_channels);
            }
            Int samples_channels = samples * src_channels;
            CopyFastN(buffer.mono + buffer.samples * src_channels, src_mono - samples_channels, samples_channels);
            buffer.samples += samples;
        }
    }
}
/******************************************************************************/
static inline void Set(I16 &sample, Flt value) {
    Int v = Round(value);
    sample = Mid(v, SHORT_MIN, SHORT_MAX);
}
static inline void Add(I16 &sample, Flt value) {
    Int v = Round(value);
    sample = Mid(sample + v, SHORT_MIN, SHORT_MAX);
}
void SoundResampler::set() { return process(Set); }
void SoundResampler::add() { return process(Add); }
/******************************************************************************/
Bool SoundResample(Int src_samples, Int src_channels, I16 *src_data, MemPtr<I16> dest_data, Flt speed, Bool hi_quality, C Flt *volume) {
    if (speed > EPS && src_samples > 0 && (src_channels == 1 || src_channels == 2) && dest_data.continuous()) {
        Int dest_samples = Round((src_samples - 1) / speed) + 1;                                    // this is needed to exactly preserve first and last src samples at the start and end of dest
        if (dest_samples > 0 && Abs(RoundL((dest_samples - 1) * speed) + 1 - src_samples) <= 65536) // check for overflow
        {
            Int dest_channels = src_channels;
            Flt vol[2];
            if (volume) {
                vol[0] = volume[0];
                vol[1] = volume[Min(1, dest_channels - 1)];
            } else {
                vol[0] = vol[1] = 1;
            }
            dest_data.setNum(dest_samples * dest_channels);
#if SUPPORT_SAMPLERATE
            if (hi_quality && speed != 1)
                if (SRC_STATE *resampler = src_new(SRC_SINC_BEST_QUALITY, src_channels, null)) {
                    Bool use_vol = (!Equal(vol[0], 1) || !Equal(vol[1], 1));
                    Flt src_flt[16384];
                    Int src_flt_samples = Elms(src_flt) / src_channels;
                    Memt<Flt> dest_flt;
                    dest_flt.setNum(Round((Elms(src_flt) + 16) / speed));
                    Flt *dest_flt_data = dest_flt.data();
                    Int dest_flt_samples = dest_flt.elms() / dest_channels;
                    Int dest_data_pos = 0;
                    SRC_DATA data;
                    data.src_ratio = 1 / speed;
                    Bool ok = true;
                    for (; src_samples > 0;) {
                        Int read = Min(src_samples, src_flt_samples);
                        data.end_of_input = (src_samples <= src_flt_samples);
                        data.data_in = src_flt;
                        data.data_out = dest_flt_data;
                        data.input_frames = read;
                        data.output_frames = dest_flt_samples;
                        Int n = read * src_channels;
                        FREP(n)
                        src_flt[i] = ShortToSFlt(src_data[i]);
                        if (src_process(resampler, &data)) {
                            ok = false;
                            break;
                        }
                        if (use_vol)
                            switch (dest_channels) {
                            case 1:
                                FREP(data.output_frames_gen)
                                dest_flt_data[i] *= vol[0];
                                break;
                            case 2:
                                FREP(data.output_frames_gen) {
                                    dest_flt_data[i * 2 + 0] *= vol[0];
                                    dest_flt_data[i * 2 + 1] *= vol[1];
                                }
                                break;
                            }
                        Int write = Min(data.output_frames_gen * dest_channels, dest_data.elms() - dest_data_pos);
                        FREP(write)
                        dest_data[dest_data_pos + i] = SFltToShort(dest_flt_data[i]);
                        dest_data_pos += write;
                        read = data.input_frames_used;
                        src_samples -= read;
                        src_data += read * src_channels;
                    }
                    src_delete(resampler);
                    if (ok) {
                        ZeroFastN(dest_data.data() + dest_data_pos, dest_data.elms() - dest_data_pos); // zero unwritten
                        return true;
                    }
                }
                // if failed then fall back to resampler below
#endif
            SoundResampler resampler(speed, vol, dest_channels, dest_samples, dest_data.data(), src_channels);
            const Int samples_step = 16384; // process up to this many samples in one step, because we need Flt precision to be reasonable for interpolation
                                            // Int si= 4096; Flt sf=si; Flt sf1=sf; IncRealByBit(sf1); sf1= 4096.00049
                                            // Int si= 8192; Flt sf=si; Flt sf1=sf; IncRealByBit(sf1); sf1= 8192.00098
                                            // Int si=16384; Flt sf=si; Flt sf1=sf; IncRealByBit(sf1); sf1=16384.0020
                                            // Int si=32768; Flt sf=si; Flt sf1=sf; IncRealByBit(sf1); sf1=32768.0039
                                            // Int si=65536; Flt sf=si; Flt sf1=sf; IncRealByBit(sf1); sf1=65536.0078
#if 0                                       // process all in one go, not good because Flt sample positions will lose precision for large data (minor precision loss at few second sounds and very noticeable for 2 minute sounds)
         resampler.setSrc(src_samples, src_data);
         resampler.set();
#elif 1                                     // limit per src (best)
            resampler.setSrc(src_samples, src_data);
            for (;;) {
                Int src_samples = resampler.src_samples;
                MIN(resampler.src_samples, samples_step);
                src_samples -= resampler.src_samples;
                resampler.end = (resampler.src_samples <= 0);
                resampler.set();
                resampler.src_samples += src_samples;
                if (resampler.dest_samples <= 0 || resampler.end && !resampler.buffer.samples)
                    break;
            }
#else // limit per dest (good, but precision depends on 'speed')
            resampler.setSrc(src_samples, src_data);
            for (;;) {
                Int dest_samples = resampler.dest_samples;
                MIN(resampler.dest_samples, samples_step);
                dest_samples -= resampler.dest_samples;
                resampler.end = (resampler.src_samples <= 0);
                resampler.set();
                resampler.dest_samples += dest_samples;
                if (resampler.dest_samples <= 0 || resampler.end && !resampler.buffer.samples)
                    break;
            }
#endif
            Int unwritten = resampler.dest_channels * resampler.dest_samples;
            ZeroFastN(dest_data.data() + (dest_data.elms() - unwritten), unwritten); // zero unwritten
            return true;
        }
    }
    dest_data.clear();
    return src_samples == 0;
}
/******************************************************************************/
} // namespace EE
/******************************************************************************/
