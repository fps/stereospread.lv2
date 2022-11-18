#include <cmath>
#include <iostream>
#include <vector>

#include <lv2.h>

#include "ir.c"

struct stereospread {
    bool is_s2s;
    float *ports[7];
    std::vector<float> buffer_l;
    std::vector<float> buffer_r;
    int buffer_head;
};

static LV2_Handle instantiate_m2s(const LV2_Descriptor *descriptor, double sample_rate, const char *bundle_path, const LV2_Feature *const *features)
{
    stereospread *instance = new stereospread;
    instance->is_s2s = false;
    instance->buffer_l = std::vector<float>(1000, 0);
    instance->buffer_head = 0; 
    return (LV2_Handle)(instance);
}

static LV2_Handle instantiate_s2s(const LV2_Descriptor *descriptor, double sample_rate, const char *bundle_path, const LV2_Feature *const *features)
{
    stereospread *instance = new stereospread;
    instance->is_s2s = true;
    instance->buffer_l = std::vector<float>(1000, 0);
    instance->buffer_r = std::vector<float>(1000, 0);
    instance->buffer_head = 0; 
    return (LV2_Handle)(instance);
}

static void cleanup(LV2_Handle instance)
{
    stereospread *tinstance = (stereospread*)instance;
    delete tinstance;
}

static void connect_port(LV2_Handle instance, uint32_t port, void *data_location)
{
    ((stereospread*)instance)->ports[port] = (float*)data_location;
}

#define EPSILON 0.0001f

static void run(LV2_Handle instance, uint32_t sample_count)
{
    stereospread *tinstance = (stereospread*)(instance);

    int control_port = 3;
    if (tinstance->is_s2s) {
        ++control_port;
    }
    const float wet = tinstance->ports[control_port++][0];
    const float dry = tinstance->ports[control_port++][0];
    const int length_port = tinstance->ports[control_port++][0];

    const int length = ir_lengths[length_port];

    const float* ir = irs[length_port];

    for(uint32_t sample_index = 0; sample_index < sample_count; ++sample_index)
    {
        if (tinstance->is_s2s == false) {
            tinstance->buffer_l[tinstance->buffer_head] = tinstance->ports[0][sample_index];
        } else {
            tinstance->buffer_l[tinstance->buffer_head] = tinstance->ports[0][sample_index];
            tinstance->buffer_r[tinstance->buffer_head] = tinstance->ports[1][sample_index];
        }

        float l = 0;
        float r = 0;

        if (tinstance->is_s2s == false) {
            for (int ir_index = 0; ir_index < length; ++ir_index) {
                l += ir[2*ir_index] * tinstance->buffer_l[(tinstance->buffer_head + ir_index) % 1000];
                r += ir[2*ir_index+1] * tinstance->buffer_l[(tinstance->buffer_head + ir_index) % 1000];
            }
        } else {
            for (int ir_index = 0; ir_index < length; ++ir_index) {
                l += ir[2*ir_index] * tinstance->buffer_l[(tinstance->buffer_head + ir_index) % 1000];
                r += ir[2*ir_index+1] * tinstance->buffer_r[(tinstance->buffer_head + ir_index) % 1000];
            }
        }

        if (tinstance->is_s2s == false) {
            tinstance->ports[1][sample_index] = wet * l + dry * tinstance->ports[0][sample_index];
            tinstance->ports[2][sample_index] = wet * r + dry * tinstance->ports[0][sample_index];
        } else {
            tinstance->ports[2][sample_index] = wet * l + dry * tinstance->ports[0][sample_index];
            tinstance->ports[3][sample_index] = wet * r + dry * tinstance->ports[1][sample_index];
        }

        --tinstance->buffer_head;
        if (tinstance->buffer_head < 0) {
            tinstance->buffer_head += 1000;
        }
    }
}

static const LV2_Descriptor descriptor_m2s = {
    "http://fps.io/plugins/stereospread-mono2stereo",
    instantiate_m2s,
    connect_port,
    nullptr, // activate
    run,
    nullptr, // deactivate
    cleanup,
    nullptr // extension_data
};

static const LV2_Descriptor descriptor_s2s = {
    "http://fps.io/plugins/stereospread-stereo2stereo",
    instantiate_s2s,
    connect_port,
    nullptr, // activate
    run,
    nullptr, // deactivate
    cleanup,
    nullptr // extension_data
};

LV2_SYMBOL_EXPORT const LV2_Descriptor* lv2_descriptor (uint32_t index)
{
    if (0 == index) return &descriptor_m2s;
    if (1 == index) return &descriptor_s2s;
    else return nullptr;
}
