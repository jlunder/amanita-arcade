#include "synth_core.h"
#include "synth_util.h"


synth_util_message_t const synth_util_message_null = {
	.timestamp = 0,
	.channel = 0,
	.command = SYNTH_UTIL_COMMAND_NULL,
};


int32_t const synth_util_velocity_map[128] = {
/*	Straightforward log velocity map -- gives very twitchy results on my
	keyboard
	
	round((math.pow(2.0, (float(i + 1) / 128.0 - 1.0) * 7)) * 65536)

	532, 552, 574, 596, 619, 643, 668, 693, 720, 748, 777, 807, 838, 870,
	904, 939, 975, 1013, 1052, 1093, 1135, 1179, 1224, 1272, 1321, 1372,
	1425, 1480, 1537, 1596, 1658, 1722, 1789, 1858, 1930, 2004, 2082, 2162,
	2245, 2332, 2422, 2516, 2613, 2714, 2819, 2928, 3041, 3158, 3280, 3407,
	3539, 3676, 3818, 3965, 4118, 4277, 4443, 4614, 4792, 4978, 5170, 5370,
	5577, 5793, 6016, 6249, 6490, 6741, 7001, 7272, 7553, 7845, 8148, 8463,
	8789, 9129, 9482, 9848, 10229, 10624, 11034, 11460, 11903, 12363, 12841,
	13337, 13852, 14387, 14943, 15520, 16120, 16743, 17390, 18061, 18759,
	19484, 20237, 21019, 21831, 22674, 23550, 24460, 25405, 26386, 27406,
	28464, 29564, 30706, 31893, 33125, 34405, 35734, 37114, 38548, 40037,
	41584, 43191, 44859, 46593, 48393, 50262, 52204, 54221, 56316, 58491,
	60751, 63098, 65536
*/
/*	Linear velocity map -- works okay in practice, but still too twitchy
	
	round((float(i + 1) / 128.0) * 65536)

	512, 1024, 1536, 2048, 2560, 3072, 3584, 4096, 4608, 5120, 5632, 6144,
	6656, 7168, 7680, 8192, 8704, 9216, 9728, 10240, 10752, 11264, 11776,
	12288, 12800, 13312, 13824, 14336, 14848, 15360, 15872, 16384, 16896,
	17408, 17920, 18432, 18944, 19456, 19968, 20480, 20992, 21504, 22016,
	22528, 23040, 23552, 24064, 24576, 25088, 25600, 26112, 26624, 27136,
	27648, 28160, 28672, 29184, 29696, 30208, 30720, 31232, 31744, 32256,
	32768, 33280, 33792, 34304, 34816, 35328, 35840, 36352, 36864, 37376,
	37888, 38400, 38912, 39424, 39936, 40448, 40960, 41472, 41984, 42496,
	43008, 43520, 44032, 44544, 45056, 45568, 46080, 46592, 47104, 47616,
	48128, 48640, 49152, 49664, 50176, 50688, 51200, 51712, 52224, 52736,
	53248, 53760, 54272, 54784, 55296, 55808, 56320, 56832, 57344, 57856,
	58368, 58880, 59392, 59904, 60416, 60928, 61440, 61952, 62464, 62976,
	63488, 64000, 64512, 65024, 65536
*/
/*	Fourth root+log velocity map -- seems to give best results
	
	round(math.pow(2.0,
		((math.pow(float(i + 1) / 128.0, 1.0 / 4.0) - 1.0) * 7)) * 65536)
*/
	2166, 2846, 3418, 3938, 4427, 4895, 5349, 5793, 6228, 6658, 7082, 7504,
	7922, 8338, 8753, 9167, 9580, 9992, 10405, 10817, 11230, 11643, 12056,
	12471, 12886, 13302, 13719, 14137, 14557, 14978, 15400, 15823, 16248,
	16674, 17102, 17532, 17963, 18395, 18829, 19265, 19703, 20142, 20584,
	21027, 21471, 21918, 22366, 22817, 23269, 23723, 24179, 24637, 25096,
	25558, 26022, 26487, 26955, 27425, 27896, 28370, 28845, 29323, 29802,
	30284, 30768, 31254, 31741, 32231, 32723, 33217, 33713, 34211, 34711,
	35214, 35718, 36224, 36733, 37244, 37756, 38271, 38788, 39308, 39829,
	40352, 40878, 41405, 41935, 42467, 43001, 43538, 44076, 44617, 45159,
	45704, 46251, 46800, 47352, 47905, 48461, 49019, 49579, 50141, 50706,
	51273, 51841, 52413, 52986, 53561, 54139, 54719, 55301, 55885, 56472,
	57060, 57651, 58244, 58840, 59437, 60037, 60639, 61244, 61850, 62459,
	63070, 63683, 64298, 64916, 65536
};


REGION_ALLOCATOR_DECLARE_MEMORY(synth_util_temp_allocator_memory, 10000);
region_allocator_t synth_util_temp_allocator;


void synth_util_init(void)
{
	region_allocator_init(&synth_util_temp_allocator,
		synth_util_temp_allocator_memory,
		sizeof synth_util_temp_allocator_memory);
}


void synth_util_shutdown(void)
{
	region_allocator_destroy(&synth_util_temp_allocator);
}


synth_util_play_rate_t synth_util_tuning_to_a440_play_rate(
	double recorded_midi_note, double sampling_rate)
{
	double a440_sample_rate = sampling_rate
		/ exp2((recorded_midi_note - SYNTH_UTIL_MIDI_NOTE_A440) / 12.0);
	return (synth_util_play_rate_t)a440_sample_rate;
}


synth_util_play_rate_t synth_util_midi_note_to_play_rate(
	synth_util_play_rate_t a440_play_rate, synth_util_midi_note_t midi_note)
{
	return synth_util_fractional_midi_note_to_play_rate(
		a440_play_rate,
		(synth_util_fractional_midi_note_t)midi_note
		* SYNTH_UTIL_FRACTIONAL_MIDI_NOTE_1);
}


synth_util_play_rate_t synth_util_fractional_midi_note_to_play_rate(
	synth_util_play_rate_t a440_play_rate,
	synth_util_fractional_midi_note_t fractional_midi_note)
{
	double midi_note = (double)fractional_midi_note
		/ (double)SYNTH_UTIL_FRACTIONAL_MIDI_NOTE_1;
	double play_rate = a440_play_rate
		* exp2((midi_note - SYNTH_UTIL_MIDI_NOTE_A440) / 12.0);
	
	return (synth_util_play_rate_t)play_rate;
}


synth_util_timespan_t synth_util_subtract_timestamps(synth_util_timestamp_t x,
	synth_util_timestamp_t y)
{
	return (synth_util_timespan_t)(x - y);
}


synth_util_timestamp_t synth_util_add_timespan_to_timestamp(
	synth_util_timestamp_t x, synth_util_timespan_t y)
{
	return x + (synth_util_timestamp_t)y;
}


synth_util_timespan_t synth_util_add_timespans(synth_util_timespan_t x,
	synth_util_timespan_t y)
{
	return x + y;
}


