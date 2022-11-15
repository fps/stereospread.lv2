#include <cmath>
#include <iostream>
#include <vector>

#include <lv2.h>

#include "ir.h"

struct dynamics {
    float *ports[6];
    std::vector<float> buffer;
    int buffer_head;
};

static LV2_Handle instantiate(const LV2_Descriptor *descriptor, double sample_rate, const char *bundle_path, const LV2_Feature *const *features)
{
    dynamics *instance = new dynamics;
    instance->buffer = std::vector<float>(1000, 0);
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

    const float wet = tinstance->ports[3][0];
    const float dry = tinstance->ports[4][0];
    const int length = tinstance->ports[5][0];

    for(uint32_t sample_index = 0; sample_index < sample_count; ++sample_index)
    {
        tinstance->buffer[tinstance->buffer_head] = tinstance->ports[0][sample_index];
        
        float l = 0;
        float r = 0;
        for (int ir_index = 0; ir_index < length; ++ir_index) {
            l += ir[2*ir_index] * tinstance->buffer[(tinstance->buffer_head + ir_index) % 1000];
            r += ir[2*ir_index+1] * tinstance->buffer[(tinstance->buffer_head + ir_index) % 1000];
        }

        tinstance->ports[1][sample_index] = wet * l + dry * tinstance->ports[0][sample_index];
        tinstance->ports[2][sample_index] = wet * r + dry * tinstance->ports[0][sample_index];

        --tinstance->buffer_head;
        if (tinstance->buffer_head < 0) {
            tinstance->buffer_head += 1000;
        }
    }
}

static const LV2_Descriptor descriptor = {
    "http://fps.io/plugins/stereospread",
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
