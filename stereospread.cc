#include <cmath>
#include <iostream>
#include <vector>

#include <lv2.h>


struct dynamics {
    float *ports[8];
    float abs1;
    float abs2;
    float samplerate;
    std::vector<float> buffer;
    int buffer_head;
};

static LV2_Handle instantiate(const LV2_Descriptor *descriptor, double sample_rate, const char *bundle_path, const LV2_Feature *const *features)
{
    dynamics *instance = new dynamics;
    instance->samplerate = sample_rate;
    instance->abs1 = 0;
    instance->abs2 = 0;
    instance->buffer = std::vector<float>(2*sample_rate, 0);
    instance->buffer_head = 0; 
    return (LV2_Handle)(instance);
}

static void cleanup(LV2_Handle instance)
{
    dynamics *tinstance = (dynamics*)instance;
    delete tinstance;
}

static void connect_port(LV2_Handle instance, uint32_t port, void *data_location)
{
    ((dynamics*)instance)->ports[port] = (float*)data_location;
}

#define EPSILON 0.0001f

static void run(LV2_Handle instance, uint32_t sample_count)
{
    dynamics *tinstance = (dynamics*)(instance);

    const float t1 = tinstance->ports[2][0] / 1000.0f;
    const float t2 = tinstance->ports[3][0] / 1000.0f;
    const float strength = tinstance->ports[4][0];
    const float delay = tinstance->ports[5][0];
    const float maxratio = tinstance->ports[6][0];
    const float minratio = tinstance->ports[7][0];

    const float a1 = 1.0f - expf((-1.0f/tinstance->samplerate) / t1);
    const float a2 = 1.0f - expf((-1.0f/tinstance->samplerate) / t2);

    for(uint32_t sample_index = 0; sample_index < sample_count; ++sample_index)
    {
        tinstance->abs1 = a1 * fabs(tinstance->ports[0][sample_index]) + (1.0f - a1) * tinstance->abs1;
        tinstance->abs2 = a2 * fabs(tinstance->ports[0][sample_index]) + (1.0f - a2) * tinstance->abs2;

        const float r = (EPSILON + tinstance->abs1) / (EPSILON + tinstance->abs2);
        float scale = powf(1.0f / r, tinstance->ports[4][0]);

        if (scale > maxratio) {
            scale = maxratio;
        }

        if (scale < minratio) {
            scale = minratio;
        }


        tinstance->buffer[tinstance->buffer_head] = tinstance->ports[0][sample_index];
        
        int buffer_tail = tinstance->buffer_head - tinstance->samplerate * (delay / 1000);
        if (buffer_tail < 0) {
            buffer_tail += 2 * tinstance->samplerate;
        }
        
        tinstance->ports[1][sample_index] = scale * tinstance->buffer[buffer_tail];
        ++tinstance->buffer_head;
        tinstance->buffer_head %= 2 * (int)tinstance->samplerate;
    }
}

static const LV2_Descriptor descriptor = {
    "http://fps.io/plugins/relative_dynamics",
    instantiate,
    connect_port,
    nullptr, // activate
    run,
    nullptr, // deactivate
    cleanup,
    nullptr // extension_data
};

LV2_SYMBOL_EXPORT const LV2_Descriptor* lv2_descriptor (uint32_t index)
{
    if (0 == index) return &descriptor;
    else return nullptr;
}
