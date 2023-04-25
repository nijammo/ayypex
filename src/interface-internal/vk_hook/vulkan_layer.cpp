/*
 * Copyright © 2019 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "../imgui_hook.h"
#include "font_default.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "vk_enum_to_str.h"
#include <fstream>
#include <inttypes.h>
#include <iostream>
#include <list>
#include <mutex>
#include <nlohmann/json.hpp>
#include <sys/stat.h>
#include <unistd.h>
#include <unordered_map>
#include <vulkan/vk_layer.h>
#include <vulkan/vk_util.h>
#include <vulkan/vulkan_core.h>

struct overlay_params {
        bool no_small_font;
        float font_size, font_scale;
        float font_size_text;
        float font_scale_media_player;
        float background_alpha, alpha;
        float cellpadding_y;
        std::string font_file, font_file_text;
        uint32_t font_glyph_ranges;
        size_t font_params_hash;
        uint32_t width, height;
};

#define VK_CHECK(expr)                                                                                                                                                                                                                                                                                     \
    do {                                                                                                                                                                                                                                                                                                   \
        VkResult __result = (expr);                                                                                                                                                                                                                                                                        \
        if (__result != VK_SUCCESS) {                                                                                                                                                                                                                                                                      \
            printf("error\n");                                                                                                                                                                                                                                                                          \
        }                                                                                                                                                                                                                                                                                                  \
    } while (0)

using namespace std;
using json = nlohmann::json;

std::mutex global_lock;
typedef std::lock_guard<std::mutex> scoped_lock;
std::unordered_map<uint64_t, void *> vk_object_to_data;

static const uint32_t overlay_vert_spv[] = {
	0x07230203,0x00010000,0x0008000b,0x0000002e,0x00000000,0x00020011,0x00000001,0x0006000b,
	0x00000001,0x4c534c47,0x6474732e,0x3035342e,0x00000000,0x0003000e,0x00000000,0x00000001,
	0x000a000f,0x00000000,0x00000004,0x6e69616d,0x00000000,0x0000000b,0x0000000f,0x00000015,
	0x0000001b,0x0000001c,0x00030003,0x00000002,0x000001c2,0x00040005,0x00000004,0x6e69616d,
	0x00000000,0x00030005,0x00000009,0x00000000,0x00050006,0x00000009,0x00000000,0x6f6c6f43,
	0x00000072,0x00040006,0x00000009,0x00000001,0x00005655,0x00030005,0x0000000b,0x0074754f,
	0x00040005,0x0000000f,0x6c6f4361,0x0000726f,0x00030005,0x00000015,0x00565561,0x00060005,
	0x00000019,0x505f6c67,0x65567265,0x78657472,0x00000000,0x00060006,0x00000019,0x00000000,
	0x505f6c67,0x7469736f,0x006e6f69,0x00030005,0x0000001b,0x00000000,0x00040005,0x0000001c,
	0x736f5061,0x00000000,0x00060005,0x0000001e,0x73755075,0x6e6f4368,0x6e617473,0x00000074,
	0x00050006,0x0000001e,0x00000000,0x61635375,0x0000656c,0x00060006,0x0000001e,0x00000001,
	0x61725475,0x616c736e,0x00006574,0x00030005,0x00000020,0x00006370,0x00040047,0x0000000b,
	0x0000001e,0x00000000,0x00040047,0x0000000f,0x0000001e,0x00000002,0x00040047,0x00000015,
	0x0000001e,0x00000001,0x00050048,0x00000019,0x00000000,0x0000000b,0x00000000,0x00030047,
	0x00000019,0x00000002,0x00040047,0x0000001c,0x0000001e,0x00000000,0x00050048,0x0000001e,
	0x00000000,0x00000023,0x00000000,0x00050048,0x0000001e,0x00000001,0x00000023,0x00000008,
	0x00030047,0x0000001e,0x00000002,0x00020013,0x00000002,0x00030021,0x00000003,0x00000002,
	0x00030016,0x00000006,0x00000020,0x00040017,0x00000007,0x00000006,0x00000004,0x00040017,
	0x00000008,0x00000006,0x00000002,0x0004001e,0x00000009,0x00000007,0x00000008,0x00040020,
	0x0000000a,0x00000003,0x00000009,0x0004003b,0x0000000a,0x0000000b,0x00000003,0x00040015,
	0x0000000c,0x00000020,0x00000001,0x0004002b,0x0000000c,0x0000000d,0x00000000,0x00040020,
	0x0000000e,0x00000001,0x00000007,0x0004003b,0x0000000e,0x0000000f,0x00000001,0x00040020,
	0x00000011,0x00000003,0x00000007,0x0004002b,0x0000000c,0x00000013,0x00000001,0x00040020,
	0x00000014,0x00000001,0x00000008,0x0004003b,0x00000014,0x00000015,0x00000001,0x00040020,
	0x00000017,0x00000003,0x00000008,0x0003001e,0x00000019,0x00000007,0x00040020,0x0000001a,
	0x00000003,0x00000019,0x0004003b,0x0000001a,0x0000001b,0x00000003,0x0004003b,0x00000014,
	0x0000001c,0x00000001,0x0004001e,0x0000001e,0x00000008,0x00000008,0x00040020,0x0000001f,
	0x00000009,0x0000001e,0x0004003b,0x0000001f,0x00000020,0x00000009,0x00040020,0x00000021,
	0x00000009,0x00000008,0x0004002b,0x00000006,0x00000028,0x00000000,0x0004002b,0x00000006,
	0x00000029,0x3f800000,0x00050036,0x00000002,0x00000004,0x00000000,0x00000003,0x000200f8,
	0x00000005,0x0004003d,0x00000007,0x00000010,0x0000000f,0x00050041,0x00000011,0x00000012,
	0x0000000b,0x0000000d,0x0003003e,0x00000012,0x00000010,0x0004003d,0x00000008,0x00000016,
	0x00000015,0x00050041,0x00000017,0x00000018,0x0000000b,0x00000013,0x0003003e,0x00000018,
	0x00000016,0x0004003d,0x00000008,0x0000001d,0x0000001c,0x00050041,0x00000021,0x00000022,
	0x00000020,0x0000000d,0x0004003d,0x00000008,0x00000023,0x00000022,0x00050085,0x00000008,
	0x00000024,0x0000001d,0x00000023,0x00050041,0x00000021,0x00000025,0x00000020,0x00000013,
	0x0004003d,0x00000008,0x00000026,0x00000025,0x00050081,0x00000008,0x00000027,0x00000024,
	0x00000026,0x00050051,0x00000006,0x0000002a,0x00000027,0x00000000,0x00050051,0x00000006,
	0x0000002b,0x00000027,0x00000001,0x00070050,0x00000007,0x0000002c,0x0000002a,0x0000002b,
	0x00000028,0x00000029,0x00050041,0x00000011,0x0000002d,0x0000001b,0x0000000d,0x0003003e,
	0x0000002d,0x0000002c,0x000100fd,0x00010038
};

static const uint32_t overlay_frag_spv[] = {
	0x07230203,0x00010000,0x0008000b,0x00000023,0x00000000,0x00020011,0x00000001,0x0006000b,
	0x00000001,0x4c534c47,0x6474732e,0x3035342e,0x00000000,0x0003000e,0x00000000,0x00000001,
	0x0007000f,0x00000004,0x00000004,0x6e69616d,0x00000000,0x00000009,0x0000000d,0x00030010,
	0x00000004,0x00000007,0x00030003,0x00000002,0x000001c2,0x00040005,0x00000004,0x6e69616d,
	0x00000000,0x00040005,0x00000009,0x6c6f4366,0x0000726f,0x00030005,0x0000000b,0x00000000,
	0x00050006,0x0000000b,0x00000000,0x6f6c6f43,0x00000072,0x00040006,0x0000000b,0x00000001,
	0x00005655,0x00030005,0x0000000d,0x00006e49,0x00050005,0x00000017,0x78655473,0x65727574,
	0x00000000,0x00040047,0x00000009,0x0000001e,0x00000000,0x00040047,0x0000000d,0x0000001e,
	0x00000000,0x00040047,0x00000017,0x00000022,0x00000000,0x00040047,0x00000017,0x00000021,
	0x00000000,0x00020013,0x00000002,0x00030021,0x00000003,0x00000002,0x00030016,0x00000006,
	0x00000020,0x00040017,0x00000007,0x00000006,0x00000004,0x00040020,0x00000008,0x00000003,
	0x00000007,0x0004003b,0x00000008,0x00000009,0x00000003,0x00040017,0x0000000a,0x00000006,
	0x00000002,0x0004001e,0x0000000b,0x00000007,0x0000000a,0x00040020,0x0000000c,0x00000001,
	0x0000000b,0x0004003b,0x0000000c,0x0000000d,0x00000001,0x00040015,0x0000000e,0x00000020,
	0x00000001,0x0004002b,0x0000000e,0x0000000f,0x00000000,0x00040020,0x00000010,0x00000001,
	0x00000007,0x0004002b,0x00000006,0x00000013,0x3f800000,0x00090019,0x00000014,0x00000006,
	0x00000001,0x00000000,0x00000000,0x00000000,0x00000001,0x00000000,0x0003001b,0x00000015,
	0x00000014,0x00040020,0x00000016,0x00000000,0x00000015,0x0004003b,0x00000016,0x00000017,
	0x00000000,0x0004002b,0x0000000e,0x00000019,0x00000001,0x00040020,0x0000001a,0x00000001,
	0x0000000a,0x00040015,0x0000001e,0x00000020,0x00000000,0x0004002b,0x0000001e,0x0000001f,
	0x00000000,0x00050036,0x00000002,0x00000004,0x00000000,0x00000003,0x000200f8,0x00000005,
	0x00050041,0x00000010,0x00000011,0x0000000d,0x0000000f,0x0004003d,0x00000007,0x00000012,
	0x00000011,0x0004003d,0x00000015,0x00000018,0x00000017,0x00050041,0x0000001a,0x0000001b,
	0x0000000d,0x00000019,0x0004003d,0x0000000a,0x0000001c,0x0000001b,0x00050057,0x00000007,
	0x0000001d,0x00000018,0x0000001c,0x00050051,0x00000006,0x00000020,0x0000001d,0x00000000,
	0x00070050,0x00000007,0x00000021,0x00000013,0x00000013,0x00000013,0x00000020,0x00050085,
	0x00000007,0x00000022,0x00000012,0x00000021,0x0003003e,0x00000009,0x00000022,0x000100fd,
	0x00010038
};


VkPhysicalDeviceDriverProperties driverProps = {};

/* Mapped from VkInstace/VkPhysicalDevice */
struct instance_data {
        struct vk_instance_dispatch_table vtable;
        VkInstance instance;
        string engineName, engineVersion;
        struct overlay_params params;
        uint32_t api_version;
};

struct command_buffer_data {
        struct device_data *device;

        VkCommandBufferLevel level;

        VkCommandBuffer cmd_buffer;

        struct queue_data *queue_data;
};

struct device_data {
        struct instance_data *instance;

        PFN_vkSetDeviceLoaderData set_device_loader_data;

        struct vk_device_dispatch_table vtable;
        VkPhysicalDevice physical_device;
        VkDevice device;

        VkPhysicalDeviceProperties properties;

        struct queue_data *graphic_queue;

        std::vector<struct queue_data *> queues;
};

struct overlay_draw {
        VkCommandBuffer command_buffer;

        VkSemaphore cross_engine_semaphore;

        VkSemaphore semaphore;
        VkFence fence;

        VkBuffer vertex_buffer;
        VkDeviceMemory vertex_buffer_mem;
        VkDeviceSize vertex_buffer_size;

        VkBuffer index_buffer;
        VkDeviceMemory index_buffer_mem;
        VkDeviceSize index_buffer_size;
};

struct swapchain_stats {
        uint64_t n_frames;
        double time_dividor;
        ImFont *font1 = nullptr;
        ImFont *font_text = nullptr;
        size_t font_params_hash = 0;
        std::string time;
        double fps;
        uint64_t last_present_time;
        unsigned n_frames_since_update;
        uint64_t last_fps_update;
        ImVec2 main_window_pos;

        struct {
                int32_t major;
                int32_t minor;
                bool is_gles;
        } version_gl;
        struct {
                int32_t major;
                int32_t minor;
                int32_t patch;
        } version_vk;
        std::string engineName;
        std::string engineVersion;
        std::string deviceName;
        std::string gpuName;
        std::string driverName;
};

struct swapchain_data {
        struct device_data *device;

        VkSwapchainKHR swapchain;
        unsigned width, height;
        VkFormat format;

        uint32_t n_images;

        std::vector<VkImage> images;
        std::vector<VkImageView> image_views;
        std::vector<VkFramebuffer> framebuffers;

        VkRenderPass render_pass;

        VkDescriptorPool descriptor_pool;
        VkDescriptorSetLayout descriptor_layout;
        VkDescriptorSet descriptor_set;

        VkSampler font_sampler;

        VkPipelineLayout pipeline_layout;
        VkPipeline pipeline;

        VkCommandPool command_pool;

        bool font_uploaded;
        VkImage font_image;
        VkImageView font_image_view;
        VkDeviceMemory font_mem;
        VkBuffer upload_font_buffer;
        VkDeviceMemory upload_font_buffer_mem;

        /**/
        ImGuiContext *imgui_context;
        ImFontAtlas *font_atlas;
        ImVec2 window_size;

        std::list<overlay_draw *> draws;

        /**/
        uint64_t n_frames;
        uint64_t last_present_time;

        unsigned n_frames_since_update;
        uint64_t last_fps_update;
        double fps;

        struct swapchain_stats sw_stats;
};

struct queue_data {
        struct device_data *device;

        VkQueue queue;
        VkQueueFlags flags;
        uint32_t family_index;
};

static uint32_t vk_memory_type(struct device_data *data, VkMemoryPropertyFlags properties, uint32_t type_bits) {
    VkPhysicalDeviceMemoryProperties prop;
    data->instance->vtable.GetPhysicalDeviceMemoryProperties(data->physical_device, &prop);
    for (uint32_t i = 0; i < prop.memoryTypeCount; i++)
        if ((prop.memoryTypes[i].propertyFlags & properties) == properties && type_bits & (1 << i))
            return i;
    return 0xFFFFFFFF; // Unable to find memoryType
}

static void *find_object_data(uint64_t obj) {
    ::scoped_lock lk(global_lock);
    return vk_object_to_data[obj];
}

static void map_object(uint64_t obj, void *data) {
    ::scoped_lock lk(global_lock);
    vk_object_to_data[obj] = data;
}

static void unmap_object(uint64_t obj) {
    ::scoped_lock lk(global_lock);
    vk_object_to_data.erase(obj);
}

#define HKEY(obj) ((uint64_t)(obj))
#define FIND(type, obj) (reinterpret_cast<type *>(find_object_data(HKEY(obj))))

static VkLayerInstanceCreateInfo *get_instance_chain_info(const VkInstanceCreateInfo *pCreateInfo, VkLayerFunction func) {
    vk_foreach_struct(item, pCreateInfo->pNext) {
        if (item->sType == VK_STRUCTURE_TYPE_LOADER_INSTANCE_CREATE_INFO && ((VkLayerInstanceCreateInfo *)item)->function == func)
            return (VkLayerInstanceCreateInfo *)item;
    }
    return NULL;
}

static struct instance_data *new_instance_data(VkInstance instance) {
    struct instance_data *data = new instance_data();
    data->instance = instance;
    map_object(HKEY(data->instance), data);
    return data;
}

static struct device_data *new_device_data(VkDevice device, struct instance_data *instance) {
    struct device_data *data = new device_data();
    data->instance = instance;
    data->device = device;
    map_object(HKEY(data->device), data);
    return data;
}

static struct command_buffer_data *new_command_buffer_data(VkCommandBuffer cmd_buffer, VkCommandBufferLevel level, struct device_data *device_data) {
    struct command_buffer_data *data = new command_buffer_data();
    data->device = device_data;
    data->cmd_buffer = cmd_buffer;
    data->level = level;
    map_object(HKEY(data->cmd_buffer), data);
    return data;
}

static void CreateOrResizeBuffer(struct device_data *data, VkBuffer *buffer, VkDeviceMemory *buffer_memory, VkDeviceSize *buffer_size, size_t new_size, VkBufferUsageFlagBits usage) {
    if (*buffer != VK_NULL_HANDLE)
        data->vtable.DestroyBuffer(data->device, *buffer, NULL);
    if (*buffer_memory)
        data->vtable.FreeMemory(data->device, *buffer_memory, NULL);

    if (data->properties.limits.nonCoherentAtomSize > 0) {
        VkDeviceSize atom_size = data->properties.limits.nonCoherentAtomSize - 1;
        new_size = (new_size + atom_size) & ~atom_size;
    }

    VkBufferCreateInfo buffer_info = {};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = new_size;
    buffer_info.usage = usage;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VK_CHECK(data->vtable.CreateBuffer(data->device, &buffer_info, NULL, buffer));

    VkMemoryRequirements req;
    data->vtable.GetBufferMemoryRequirements(data->device, *buffer, &req);
    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = req.size;
    alloc_info.memoryTypeIndex = vk_memory_type(data, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, req.memoryTypeBits);
    VK_CHECK(data->vtable.AllocateMemory(data->device, &alloc_info, NULL, buffer_memory));

    VK_CHECK(data->vtable.BindBufferMemory(data->device, *buffer, *buffer_memory, 0));
    *buffer_size = new_size;
}

static void shutdown_swapchain_font(struct swapchain_data *data) {
    struct device_data *device_data = data->device;

    device_data->vtable.DestroyImageView(device_data->device, data->font_image_view, NULL);
    device_data->vtable.DestroyImage(device_data->device, data->font_image, NULL);
    device_data->vtable.FreeMemory(device_data->device, data->font_mem, NULL);

    device_data->vtable.DestroyBuffer(device_data->device, data->upload_font_buffer, NULL);
    device_data->vtable.FreeMemory(device_data->device, data->upload_font_buffer_mem, NULL);
}

static void update_image_descriptor(struct swapchain_data *data, VkImageView image_view, VkDescriptorSet set) {
    struct device_data *device_data = data->device;
    /* Descriptor set */
    VkDescriptorImageInfo desc_image[1] = {};
    desc_image[0].sampler = data->font_sampler;
    desc_image[0].imageView = image_view;
    desc_image[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    VkWriteDescriptorSet write_desc[1] = {};
    write_desc[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write_desc[0].dstSet = set;
    write_desc[0].descriptorCount = 1;
    write_desc[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write_desc[0].pImageInfo = desc_image;
    device_data->vtable.UpdateDescriptorSets(device_data->device, 1, write_desc, 0, NULL);
}

static void create_image(struct swapchain_data *data, VkDescriptorSet descriptor_set, uint32_t width, uint32_t height, VkFormat format, VkImage &image, VkDeviceMemory &image_mem, VkImageView &image_view) {
    struct device_data *device_data = data->device;

    VkImageCreateInfo image_info = {};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.format = format;
    image_info.extent.width = width;
    image_info.extent.height = height;
    image_info.extent.depth = 1;
    image_info.mipLevels = 1;
    image_info.arrayLayers = 1;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VK_CHECK(device_data->vtable.CreateImage(device_data->device, &image_info, NULL, &image));
    VkMemoryRequirements font_image_req;
    device_data->vtable.GetImageMemoryRequirements(device_data->device, image, &font_image_req);
    VkMemoryAllocateInfo image_alloc_info = {};
    image_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    image_alloc_info.allocationSize = font_image_req.size;
    image_alloc_info.memoryTypeIndex = vk_memory_type(device_data, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, font_image_req.memoryTypeBits);
    VK_CHECK(device_data->vtable.AllocateMemory(device_data->device, &image_alloc_info, NULL, &image_mem));
    VK_CHECK(device_data->vtable.BindImageMemory(device_data->device, image, image_mem, 0));

    /* Font image view */
    VkImageViewCreateInfo view_info = {};
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.image = image;
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = format;
    view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.layerCount = 1;
    VK_CHECK(device_data->vtable.CreateImageView(device_data->device, &view_info, NULL, &image_view));

    update_image_descriptor(data, image_view, descriptor_set);
}

static VkDescriptorSet create_image_with_desc(struct swapchain_data *data, uint32_t width, uint32_t height, VkFormat format, VkImage &image, VkDeviceMemory &image_mem, VkImageView &image_view) {
    struct device_data *device_data = data->device;

    VkDescriptorSet descriptor_set{};

    VkDescriptorSetAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = data->descriptor_pool;
    alloc_info.descriptorSetCount = 1;
    alloc_info.pSetLayouts = &data->descriptor_layout;
    VK_CHECK(device_data->vtable.AllocateDescriptorSets(device_data->device, &alloc_info, &descriptor_set));

    create_image(data, descriptor_set, width, height, format, image, image_mem, image_view);
    return descriptor_set;
}

template <typename... Ts> size_t get_hash(Ts const &...args) {
    size_t hash = 0;
    ((hash ^= std::hash<Ts>{}(args) << 1), ...);
    return hash;
}

static void check_fonts(struct swapchain_data *data) {
    struct device_data *device_data = data->device;
    struct instance_data *instance_data = device_data->instance;
    auto &params = instance_data->params;

    // this is needed. WHY.
    params.font_file = "?!?!";
    params.font_size = 14;
    params.font_scale = 1;
    params.font_size_text = 14;

    params.font_params_hash = get_hash(params.font_size, params.font_size_text, params.no_small_font, params.font_file, params.font_file_text, params.font_glyph_ranges);

    if (params.font_params_hash != data->sw_stats.font_params_hash) {
        VkDescriptorSet desc_set = (VkDescriptorSet)data->font_atlas->TexID;
        create_fonts(data->font_atlas, data->sw_stats.font1, data->sw_stats.font_text);
        unsigned char *pixels;
        int width, height;
        data->font_atlas->GetTexDataAsAlpha8(&pixels, &width, &height);
        device_data->vtable.DeviceWaitIdle(device_data->device);
        shutdown_swapchain_font(data);

        if (desc_set)
            create_image(data, desc_set, width, height, VK_FORMAT_R8_UNORM, data->font_image, data->font_mem, data->font_image_view);
        else
            desc_set = create_image_with_desc(data, width, height, VK_FORMAT_R8_UNORM, data->font_image, data->font_mem, data->font_image_view);

        data->font_atlas->SetTexID((ImTextureID)desc_set);
        ImGui::GetIO().Fonts = data->font_atlas;
        data->font_uploaded = false;
        data->sw_stats.font_params_hash = params.font_params_hash;
    }
}

static void upload_image_data(struct device_data *device_data, VkCommandBuffer command_buffer, void *pixels, VkDeviceSize upload_size, uint32_t width, uint32_t height, VkBuffer &upload_buffer, VkDeviceMemory &upload_buffer_mem, VkImage image) {
    /* Upload buffer */
    VkBufferCreateInfo buffer_info = {};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = upload_size;
    buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VK_CHECK(device_data->vtable.CreateBuffer(device_data->device, &buffer_info, NULL, &upload_buffer));
    VkMemoryRequirements upload_buffer_req;
    device_data->vtable.GetBufferMemoryRequirements(device_data->device, upload_buffer, &upload_buffer_req);
    VkMemoryAllocateInfo upload_alloc_info = {};
    upload_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    upload_alloc_info.allocationSize = upload_buffer_req.size;
    upload_alloc_info.memoryTypeIndex = vk_memory_type(device_data, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, upload_buffer_req.memoryTypeBits);
    VK_CHECK(device_data->vtable.AllocateMemory(device_data->device, &upload_alloc_info, NULL, &upload_buffer_mem));
    VK_CHECK(device_data->vtable.BindBufferMemory(device_data->device, upload_buffer, upload_buffer_mem, 0));

    /* Upload to Buffer */
    char *map = NULL;
    VK_CHECK(device_data->vtable.MapMemory(device_data->device, upload_buffer_mem, 0, upload_size, 0, (void **)(&map)));
    memcpy(map, pixels, upload_size);
    VkMappedMemoryRange range[1] = {};
    range[0].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    range[0].memory = upload_buffer_mem;
    range[0].size = upload_size;
    VK_CHECK(device_data->vtable.FlushMappedMemoryRanges(device_data->device, 1, range));
    device_data->vtable.UnmapMemory(device_data->device, upload_buffer_mem);

    /* Copy buffer to image */
    VkImageMemoryBarrier copy_barrier[1] = {};
    copy_barrier[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    copy_barrier[0].dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    copy_barrier[0].oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    copy_barrier[0].newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    copy_barrier[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    copy_barrier[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    copy_barrier[0].image = image;
    copy_barrier[0].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_barrier[0].subresourceRange.levelCount = 1;
    copy_barrier[0].subresourceRange.layerCount = 1;
    device_data->vtable.CmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, copy_barrier);

    VkBufferImageCopy region = {};
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.layerCount = 1;
    region.imageExtent.width = width;
    region.imageExtent.height = height;
    region.imageExtent.depth = 1;
    device_data->vtable.CmdCopyBufferToImage(command_buffer, upload_buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    VkImageMemoryBarrier use_barrier[1] = {};
    use_barrier[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    use_barrier[0].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    use_barrier[0].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    use_barrier[0].oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    use_barrier[0].newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    use_barrier[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    use_barrier[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    use_barrier[0].image = image;
    use_barrier[0].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    use_barrier[0].subresourceRange.levelCount = 1;
    use_barrier[0].subresourceRange.layerCount = 1;
    device_data->vtable.CmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, 1, use_barrier);
}

static void ensure_swapchain_fonts(struct swapchain_data *data, VkCommandBuffer command_buffer) {
    struct device_data *device_data = data->device;

    check_fonts(data);

    if (data->font_uploaded)
        return;

    data->font_uploaded = true;
    unsigned char *pixels;
    int width, height;
    data->font_atlas->GetTexDataAsAlpha8(&pixels, &width, &height);
    size_t upload_size = width * height * 1 * sizeof(char);
    upload_image_data(device_data, command_buffer, pixels, upload_size, width, height, data->upload_font_buffer, data->upload_font_buffer_mem, data->font_image);
}

struct overlay_draw *get_overlay_draw(struct swapchain_data *data) {
    struct device_data *device_data = data->device;
    struct overlay_draw *draw = data->draws.empty() ? nullptr : data->draws.front();

    VkSemaphoreCreateInfo sem_info = {};
    sem_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    if (draw && device_data->vtable.GetFenceStatus(device_data->device, draw->fence) == VK_SUCCESS) {
        VK_CHECK(device_data->vtable.ResetFences(device_data->device, 1, &draw->fence));
        data->draws.pop_front();
        data->draws.push_back(draw);
        return draw;
    }

    draw = new overlay_draw();

    VkCommandBufferAllocateInfo cmd_buffer_info = {};
    cmd_buffer_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmd_buffer_info.commandPool = data->command_pool;
    cmd_buffer_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmd_buffer_info.commandBufferCount = 1;
    VK_CHECK(device_data->vtable.AllocateCommandBuffers(device_data->device, &cmd_buffer_info, &draw->command_buffer));
    VK_CHECK(device_data->set_device_loader_data(device_data->device, draw->command_buffer));

    VkFenceCreateInfo fence_info = {};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    VK_CHECK(device_data->vtable.CreateFence(device_data->device, &fence_info, NULL, &draw->fence));

    VK_CHECK(device_data->vtable.CreateSemaphore(device_data->device, &sem_info, NULL, &draw->semaphore));
    VK_CHECK(device_data->vtable.CreateSemaphore(device_data->device, &sem_info, NULL, &draw->cross_engine_semaphore));

    data->draws.push_back(draw);

    return draw;
}

static struct overlay_draw *render_swapchain_display(struct swapchain_data *data, struct queue_data *present_queue, const VkSemaphore *wait_semaphores, unsigned n_wait_semaphores, unsigned image_index) {
    ImDrawData *draw_data = ImGui::GetDrawData();
    struct device_data *device_data = data->device;

    if (!draw_data || draw_data->TotalVtxCount == 0)
        return nullptr;

    struct overlay_draw *draw = get_overlay_draw(data);

    device_data->vtable.ResetCommandBuffer(draw->command_buffer, 0);

    VkRenderPassBeginInfo render_pass_info = {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = data->render_pass;
    render_pass_info.framebuffer = data->framebuffers[image_index];
    render_pass_info.renderArea.extent.width = data->width;
    render_pass_info.renderArea.extent.height = data->height;

    VkCommandBufferBeginInfo buffer_begin_info = {};
    buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    device_data->vtable.BeginCommandBuffer(draw->command_buffer, &buffer_begin_info);

    ensure_swapchain_fonts(data, draw->command_buffer);

    /* Bounce the image to display back to color attachment layout for
     * rendering on top of it.
     */
    VkImageMemoryBarrier imb;
    imb.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imb.pNext = nullptr;
    imb.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    imb.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    imb.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    imb.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    imb.image = data->images[image_index];
    imb.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imb.subresourceRange.baseMipLevel = 0;
    imb.subresourceRange.levelCount = 1;
    imb.subresourceRange.baseArrayLayer = 0;
    imb.subresourceRange.layerCount = 1;
    imb.srcQueueFamilyIndex = present_queue->family_index;
    imb.dstQueueFamilyIndex = device_data->graphic_queue->family_index;
    device_data->vtable.CmdPipelineBarrier(draw->command_buffer, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, 0, /* dependency flags */
                                           0, nullptr,                                                                                      /* memory barriers */
                                           0, nullptr,                                                                                      /* buffer memory barriers */
                                           1, &imb);                                                                                        /* image memory barriers */

    device_data->vtable.CmdBeginRenderPass(draw->command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

    /* Create/Resize vertex & index buffers */
    size_t vertex_size = draw_data->TotalVtxCount * sizeof(ImDrawVert);
    size_t index_size = draw_data->TotalIdxCount * sizeof(ImDrawIdx);
    if (draw->vertex_buffer_size < vertex_size) {
        CreateOrResizeBuffer(device_data, &draw->vertex_buffer, &draw->vertex_buffer_mem, &draw->vertex_buffer_size, vertex_size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    }
    if (draw->index_buffer_size < index_size) {
        CreateOrResizeBuffer(device_data, &draw->index_buffer, &draw->index_buffer_mem, &draw->index_buffer_size, index_size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    }

    /* Upload vertex & index data */
    ImDrawVert *vtx_dst = NULL;
    ImDrawIdx *idx_dst = NULL;
    VK_CHECK(device_data->vtable.MapMemory(device_data->device, draw->vertex_buffer_mem, 0, draw->vertex_buffer_size, 0, (void **)(&vtx_dst)));
    VK_CHECK(device_data->vtable.MapMemory(device_data->device, draw->index_buffer_mem, 0, draw->index_buffer_size, 0, (void **)(&idx_dst)));
    for (int n = 0; n < draw_data->CmdListsCount; n++) {
        const ImDrawList *cmd_list = draw_data->CmdLists[n];
        memcpy(vtx_dst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
        memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
        vtx_dst += cmd_list->VtxBuffer.Size;
        idx_dst += cmd_list->IdxBuffer.Size;
    }
    VkMappedMemoryRange range[2] = {};
    range[0].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    range[0].memory = draw->vertex_buffer_mem;
    range[0].size = VK_WHOLE_SIZE;
    range[1].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    range[1].memory = draw->index_buffer_mem;
    range[1].size = VK_WHOLE_SIZE;
    VK_CHECK(device_data->vtable.FlushMappedMemoryRanges(device_data->device, 2, range));
    device_data->vtable.UnmapMemory(device_data->device, draw->vertex_buffer_mem);
    device_data->vtable.UnmapMemory(device_data->device, draw->index_buffer_mem);

    /* Bind pipeline and descriptor sets */
    device_data->vtable.CmdBindPipeline(draw->command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, data->pipeline);

#if 1                              // disable if using >1 font textures
    VkDescriptorSet desc_set[1] = {// data->descriptor_set
                                   reinterpret_cast<VkDescriptorSet>(data->font_atlas->TexID)};
    device_data->vtable.CmdBindDescriptorSets(draw->command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, data->pipeline_layout, 0, 1, desc_set, 0, NULL);
#endif

    /* Bind vertex & index buffers */
    VkBuffer vertex_buffers[1] = {draw->vertex_buffer};
    VkDeviceSize vertex_offset[1] = {0};
    device_data->vtable.CmdBindVertexBuffers(draw->command_buffer, 0, 1, vertex_buffers, vertex_offset);
    device_data->vtable.CmdBindIndexBuffer(draw->command_buffer, draw->index_buffer, 0, VK_INDEX_TYPE_UINT16);

    /* Setup viewport */
    VkViewport viewport;
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = draw_data->DisplaySize.x;
    viewport.height = draw_data->DisplaySize.y;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    device_data->vtable.CmdSetViewport(draw->command_buffer, 0, 1, &viewport);

    /* Setup scale and translation through push constants :
     *
     * Our visible imgui space lies from draw_data->DisplayPos (top left) to
     * draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayMin
     * is typically (0,0) for single viewport apps.
     */
    float scale[2];
    scale[0] = 2.0f / draw_data->DisplaySize.x;
    scale[1] = 2.0f / draw_data->DisplaySize.y;
    float translate[2];
    translate[0] = -1.0f - draw_data->DisplayPos.x * scale[0];
    translate[1] = -1.0f - draw_data->DisplayPos.y * scale[1];
    device_data->vtable.CmdPushConstants(draw->command_buffer, data->pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, sizeof(float) * 0, sizeof(float) * 2, scale);
    device_data->vtable.CmdPushConstants(draw->command_buffer, data->pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, sizeof(float) * 2, sizeof(float) * 2, translate);

    // Render the command lists:
    int vtx_offset = 0;
    int idx_offset = 0;
    ImVec2 display_pos = draw_data->DisplayPos;
    for (int n = 0; n < draw_data->CmdListsCount; n++) {
        const ImDrawList *cmd_list = draw_data->CmdLists[n];
        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
            const ImDrawCmd *pcmd = &cmd_list->CmdBuffer[cmd_i];
            // Apply scissor/clipping rectangle
            // FIXME: We could clamp width/height based on clamped min/max
            // values.
            VkRect2D scissor;
            scissor.offset.x = (int32_t)(pcmd->ClipRect.x - display_pos.x) > 0 ? (int32_t)(pcmd->ClipRect.x - display_pos.x) : 0;
            scissor.offset.y = (int32_t)(pcmd->ClipRect.y - display_pos.y) > 0 ? (int32_t)(pcmd->ClipRect.y - display_pos.y) : 0;
            scissor.extent.width = (uint32_t)(pcmd->ClipRect.z - pcmd->ClipRect.x);
            scissor.extent.height = (uint32_t)(pcmd->ClipRect.w - pcmd->ClipRect.y + 1); // FIXME: Why +1 here?
            device_data->vtable.CmdSetScissor(draw->command_buffer, 0, 1, &scissor);
#if 0 // enable if using >1 font textures or use texture array
         VkDescriptorSet desc_set[1] = { (VkDescriptorSet)pcmd->TextureId };
         device_data->vtable.CmdBindDescriptorSets(draw->command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                                   data->pipeline_layout, 0, 1, desc_set, 0, NULL);
#endif
            // Draw
            device_data->vtable.CmdDrawIndexed(draw->command_buffer, pcmd->ElemCount, 1, idx_offset, vtx_offset, 0);

            idx_offset += pcmd->ElemCount;
        }
        vtx_offset += cmd_list->VtxBuffer.Size;
    }

    device_data->vtable.CmdEndRenderPass(draw->command_buffer);

    if (device_data->graphic_queue->family_index != present_queue->family_index) {
        /* Transfer the image back to the present queue family
         * image layout was already changed to present by the render pass
         */
        imb.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imb.pNext = nullptr;
        imb.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        imb.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        imb.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        imb.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        imb.image = data->images[image_index];
        imb.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imb.subresourceRange.baseMipLevel = 0;
        imb.subresourceRange.levelCount = 1;
        imb.subresourceRange.baseArrayLayer = 0;
        imb.subresourceRange.layerCount = 1;
        imb.srcQueueFamilyIndex = device_data->graphic_queue->family_index;
        imb.dstQueueFamilyIndex = present_queue->family_index;
        device_data->vtable.CmdPipelineBarrier(draw->command_buffer, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, 0, /* dependency flags */
                                               0, nullptr,                                                                                      /* memory barriers */
                                               0, nullptr,                                                                                      /* buffer memory barriers */
                                               1, &imb);                                                                                        /* image memory barriers */
    }

    device_data->vtable.EndCommandBuffer(draw->command_buffer);

    /* When presenting on a different queue than where we're drawing the
     * overlay *AND* when the application does not provide a semaphore to
     * vkQueuePresent, insert our own cross engine synchronization
     * semaphore.
     */
    if (n_wait_semaphores == 0 && device_data->graphic_queue->queue != present_queue->queue) {
        VkPipelineStageFlags stages_wait = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        VkSubmitInfo submit_info = {};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 0;
        submit_info.pWaitDstStageMask = &stages_wait;
        submit_info.waitSemaphoreCount = 0;
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = &draw->cross_engine_semaphore;

        device_data->vtable.QueueSubmit(present_queue->queue, 1, &submit_info, VK_NULL_HANDLE);

        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pWaitDstStageMask = &stages_wait;
        submit_info.pCommandBuffers = &draw->command_buffer;
        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = &draw->cross_engine_semaphore;
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = &draw->semaphore;

        device_data->vtable.QueueSubmit(device_data->graphic_queue->queue, 1, &submit_info, draw->fence);
    } else {
        // wait in the fragment stage until the swapchain image is ready
        std::vector<VkPipelineStageFlags> stages_wait(n_wait_semaphores, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

        VkSubmitInfo submit_info = {};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &draw->command_buffer;
        submit_info.pWaitDstStageMask = stages_wait.data();
        submit_info.waitSemaphoreCount = n_wait_semaphores;
        submit_info.pWaitSemaphores = wait_semaphores;
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = &draw->semaphore;

        device_data->vtable.QueueSubmit(device_data->graphic_queue->queue, 1, &submit_info, draw->fence);
    }

    return draw;
}

static void convert_colors_vk(VkFormat format, struct swapchain_stats &sw_stats, struct overlay_params &params) {
    switch (format) {
    case VK_FORMAT_R8_SRGB:
    case VK_FORMAT_R8G8_SRGB:
    case VK_FORMAT_R8G8B8_SRGB:
    case VK_FORMAT_B8G8R8_SRGB:
    case VK_FORMAT_R8G8B8A8_SRGB:
    case VK_FORMAT_B8G8R8A8_SRGB:
    case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
    case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
    case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
    case VK_FORMAT_BC2_SRGB_BLOCK:
    case VK_FORMAT_BC3_SRGB_BLOCK:
    case VK_FORMAT_BC7_SRGB_BLOCK:
    case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK:
    case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK:
    case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK:
    case VK_FORMAT_ASTC_4x4_SRGB_BLOCK:
    case VK_FORMAT_ASTC_5x4_SRGB_BLOCK:
    case VK_FORMAT_ASTC_5x5_SRGB_BLOCK:
    case VK_FORMAT_ASTC_6x5_SRGB_BLOCK:
    case VK_FORMAT_ASTC_6x6_SRGB_BLOCK:
    case VK_FORMAT_ASTC_8x5_SRGB_BLOCK:
    case VK_FORMAT_ASTC_8x6_SRGB_BLOCK:
    case VK_FORMAT_ASTC_8x8_SRGB_BLOCK:
    case VK_FORMAT_ASTC_10x5_SRGB_BLOCK:
    case VK_FORMAT_ASTC_10x6_SRGB_BLOCK:
    case VK_FORMAT_ASTC_10x8_SRGB_BLOCK:
    case VK_FORMAT_ASTC_10x10_SRGB_BLOCK:
    case VK_FORMAT_ASTC_12x10_SRGB_BLOCK:
    case VK_FORMAT_ASTC_12x12_SRGB_BLOCK:
    case VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG:
    case VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG:
    case VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG:
    case VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG:
        break;
    default:
        break;
    }

    // HUDElements.convert_colors(do_conv, params);
}

static void setup_swapchain_data_pipeline(struct swapchain_data *data) {
    struct device_data *device_data = data->device;
    VkShaderModule vert_module, frag_module;

    /* Create shader modules */
    VkShaderModuleCreateInfo vert_info = {};
    vert_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vert_info.codeSize = sizeof(overlay_vert_spv);
    vert_info.pCode = overlay_vert_spv;
    VK_CHECK(device_data->vtable.CreateShaderModule(device_data->device, &vert_info, NULL, &vert_module));
    VkShaderModuleCreateInfo frag_info = {};
    frag_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    frag_info.codeSize = sizeof(overlay_frag_spv);
    frag_info.pCode = (uint32_t *)overlay_frag_spv;
    VK_CHECK(device_data->vtable.CreateShaderModule(device_data->device, &frag_info, NULL, &frag_module));

    /* Font sampler */
    VkSamplerCreateInfo sampler_info = {};
    sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_info.magFilter = VK_FILTER_LINEAR;
    sampler_info.minFilter = VK_FILTER_LINEAR;
    sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.minLod = -1000;
    sampler_info.maxLod = 1000;
    sampler_info.maxAnisotropy = 1.0f;
    VK_CHECK(device_data->vtable.CreateSampler(device_data->device, &sampler_info, NULL, &data->font_sampler));

    /* Descriptor pool */
    VkDescriptorPoolSize sampler_pool_size = {};
    sampler_pool_size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    sampler_pool_size.descriptorCount = 1;
    VkDescriptorPoolCreateInfo desc_pool_info = {};
    desc_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    desc_pool_info.maxSets = 1;
    desc_pool_info.poolSizeCount = 1;
    desc_pool_info.pPoolSizes = &sampler_pool_size;
    VK_CHECK(device_data->vtable.CreateDescriptorPool(device_data->device, &desc_pool_info, NULL, &data->descriptor_pool));

    /* Descriptor layout */
    VkSampler sampler[1] = {data->font_sampler};
    VkDescriptorSetLayoutBinding binding[1] = {};
    binding[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    binding[0].descriptorCount = 1;
    binding[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    binding[0].pImmutableSamplers = sampler;
    VkDescriptorSetLayoutCreateInfo set_layout_info = {};
    set_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    set_layout_info.bindingCount = 1;
    set_layout_info.pBindings = binding;
    VK_CHECK(device_data->vtable.CreateDescriptorSetLayout(device_data->device, &set_layout_info, NULL, &data->descriptor_layout));

    /* Descriptor set */
    /*
       VkDescriptorSetAllocateInfo alloc_info = {};
       alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
       alloc_info.descriptorPool = data->descriptor_pool;
       alloc_info.descriptorSetCount = 1;
       alloc_info.pSetLayouts = &data->descriptor_layout;
       VK_CHECK(device_data->vtable.AllocateDescriptorSets(device_data->device,
                                                           &alloc_info,
                                                           &data->descriptor_set));
    */

    /* Constants: we are using 'vec2 offset' and 'vec2 scale' instead of a full
     * 3d projection matrix
     */
    VkPushConstantRange push_constants[1] = {};
    push_constants[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    push_constants[0].offset = sizeof(float) * 0;
    push_constants[0].size = sizeof(float) * 4;
    VkPipelineLayoutCreateInfo layout_info = {};
    layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layout_info.setLayoutCount = 1;
    layout_info.pSetLayouts = &data->descriptor_layout;
    layout_info.pushConstantRangeCount = 1;
    layout_info.pPushConstantRanges = push_constants;
    VK_CHECK(device_data->vtable.CreatePipelineLayout(device_data->device, &layout_info, NULL, &data->pipeline_layout));

    VkPipelineShaderStageCreateInfo stage[2] = {};
    stage[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stage[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    stage[0].module = vert_module;
    stage[0].pName = "main";
    stage[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stage[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    stage[1].module = frag_module;
    stage[1].pName = "main";

    VkVertexInputBindingDescription binding_desc[1] = {};
    binding_desc[0].stride = sizeof(ImDrawVert);
    binding_desc[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription attribute_desc[3] = {};
    attribute_desc[0].location = 0;
    attribute_desc[0].binding = binding_desc[0].binding;
    attribute_desc[0].format = VK_FORMAT_R32G32_SFLOAT;
    attribute_desc[0].offset = IM_OFFSETOF(ImDrawVert, pos);
    attribute_desc[1].location = 1;
    attribute_desc[1].binding = binding_desc[0].binding;
    attribute_desc[1].format = VK_FORMAT_R32G32_SFLOAT;
    attribute_desc[1].offset = IM_OFFSETOF(ImDrawVert, uv);
    attribute_desc[2].location = 2;
    attribute_desc[2].binding = binding_desc[0].binding;
    attribute_desc[2].format = VK_FORMAT_R8G8B8A8_UNORM;
    attribute_desc[2].offset = IM_OFFSETOF(ImDrawVert, col);

    VkPipelineVertexInputStateCreateInfo vertex_info = {};
    vertex_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_info.vertexBindingDescriptionCount = 1;
    vertex_info.pVertexBindingDescriptions = binding_desc;
    vertex_info.vertexAttributeDescriptionCount = 3;
    vertex_info.pVertexAttributeDescriptions = attribute_desc;

    VkPipelineInputAssemblyStateCreateInfo ia_info = {};
    ia_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    ia_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkPipelineViewportStateCreateInfo viewport_info = {};
    viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_info.viewportCount = 1;
    viewport_info.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo raster_info = {};
    raster_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    raster_info.polygonMode = VK_POLYGON_MODE_FILL;
    raster_info.cullMode = VK_CULL_MODE_NONE;
    raster_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    raster_info.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo ms_info = {};
    ms_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    ms_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState color_attachment[1] = {};
    color_attachment[0].blendEnable = VK_TRUE;
    color_attachment[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    color_attachment[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_attachment[0].colorBlendOp = VK_BLEND_OP_ADD;
    color_attachment[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    color_attachment[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_attachment[0].alphaBlendOp = VK_BLEND_OP_ADD;
    color_attachment[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    VkPipelineDepthStencilStateCreateInfo depth_info = {};
    depth_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

    VkPipelineColorBlendStateCreateInfo blend_info = {};
    blend_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    blend_info.attachmentCount = 1;
    blend_info.pAttachments = color_attachment;

    VkDynamicState dynamic_states[2] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamic_state = {};
    dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state.dynamicStateCount = (uint32_t)IM_ARRAYSIZE(dynamic_states);
    dynamic_state.pDynamicStates = dynamic_states;

    VkGraphicsPipelineCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    info.flags = 0;
    info.stageCount = 2;
    info.pStages = stage;
    info.pVertexInputState = &vertex_info;
    info.pInputAssemblyState = &ia_info;
    info.pViewportState = &viewport_info;
    info.pRasterizationState = &raster_info;
    info.pMultisampleState = &ms_info;
    info.pDepthStencilState = &depth_info;
    info.pColorBlendState = &blend_info;
    info.pDynamicState = &dynamic_state;
    info.layout = data->pipeline_layout;
    info.renderPass = data->render_pass;
    VK_CHECK(device_data->vtable.CreateGraphicsPipelines(device_data->device, VK_NULL_HANDLE, 1, &info, NULL, &data->pipeline));

    device_data->vtable.DestroyShaderModule(device_data->device, vert_module, NULL);
    device_data->vtable.DestroyShaderModule(device_data->device, frag_module, NULL);

    check_fonts(data);
}

static void setup_swapchain_data(struct swapchain_data *data, const VkSwapchainCreateInfoKHR *pCreateInfo) {
    struct device_data *device_data = data->device;
    data->width = pCreateInfo->imageExtent.width;
    data->height = pCreateInfo->imageExtent.height;
    data->format = pCreateInfo->imageFormat;

    data->imgui_context = ImGui::CreateContext();
    ImGui::SetCurrentContext(data->imgui_context);

    ImGui::GetIO().IniFilename = NULL;
    ImGui::GetIO().DisplaySize = ImVec2((float)data->width, (float)data->height);

    convert_colors_vk(pCreateInfo->imageFormat, data->sw_stats, device_data->instance->params);

    /* Render pass */
    VkAttachmentDescription attachment_desc = {};
    attachment_desc.format = pCreateInfo->imageFormat;
    attachment_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment_desc.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    attachment_desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment_desc.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachment_desc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    VkAttachmentReference color_attachment = {};
    color_attachment.attachment = 0;
    color_attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment;
    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    VkRenderPassCreateInfo render_pass_info = {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = 1;
    render_pass_info.pAttachments = &attachment_desc;
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;
    render_pass_info.dependencyCount = 1;
    render_pass_info.pDependencies = &dependency;
    VK_CHECK(device_data->vtable.CreateRenderPass(device_data->device, &render_pass_info, NULL, &data->render_pass));

    setup_swapchain_data_pipeline(data);

    uint32_t n_images = 0;
    VK_CHECK(device_data->vtable.GetSwapchainImagesKHR(device_data->device, data->swapchain, &n_images, NULL));

    data->images.resize(n_images);
    data->image_views.resize(n_images);
    data->framebuffers.resize(n_images);

    VK_CHECK(device_data->vtable.GetSwapchainImagesKHR(device_data->device, data->swapchain, &n_images, data->images.data()));

    if (n_images != data->images.size()) {
        data->images.resize(n_images);
        data->image_views.resize(n_images);
        data->framebuffers.resize(n_images);
    }

    /* Image views */
    VkImageViewCreateInfo view_info = {};
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = pCreateInfo->imageFormat;
    view_info.components.r = VK_COMPONENT_SWIZZLE_R;
    view_info.components.g = VK_COMPONENT_SWIZZLE_G;
    view_info.components.b = VK_COMPONENT_SWIZZLE_B;
    view_info.components.a = VK_COMPONENT_SWIZZLE_A;
    view_info.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    for (size_t i = 0; i < data->images.size(); i++) {
        view_info.image = data->images[i];
        VK_CHECK(device_data->vtable.CreateImageView(device_data->device, &view_info, NULL, &data->image_views[i]));
    }

    /* Framebuffers */
    VkImageView attachment[1];
    VkFramebufferCreateInfo fb_info = {};
    fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fb_info.renderPass = data->render_pass;
    fb_info.attachmentCount = 1;
    fb_info.pAttachments = attachment;
    fb_info.width = data->width;
    fb_info.height = data->height;
    fb_info.layers = 1;
    for (size_t i = 0; i < data->image_views.size(); i++) {
        attachment[0] = data->image_views[i];
        VK_CHECK(device_data->vtable.CreateFramebuffer(device_data->device, &fb_info, NULL, &data->framebuffers[i]));
    }
    /* Command buffer pool */
    VkCommandPoolCreateInfo cmd_buffer_pool_info = {};
    cmd_buffer_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmd_buffer_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    cmd_buffer_pool_info.queueFamilyIndex = device_data->graphic_queue->family_index;

    VK_CHECK(device_data->vtable.CreateCommandPool(device_data->device, &cmd_buffer_pool_info, NULL, &data->command_pool));
}

static void instance_data_map_physical_devices(struct instance_data *instance_data, bool map) {
    uint32_t physicalDeviceCount = 0;
    instance_data->vtable.EnumeratePhysicalDevices(instance_data->instance, &physicalDeviceCount, NULL);

    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
    instance_data->vtable.EnumeratePhysicalDevices(instance_data->instance, &physicalDeviceCount, physicalDevices.data());

    for (uint32_t i = 0; i < physicalDeviceCount; i++) {
        if (map)
            map_object(HKEY(physicalDevices[i]), instance_data);
        else
            unmap_object(HKEY(physicalDevices[i]));
    }
}

static struct swapchain_data *new_swapchain_data(VkSwapchainKHR swapchain, struct device_data *device_data) {
    struct instance_data *instance_data = device_data->instance;
    struct swapchain_data *data = new swapchain_data();
    data->device = device_data;
    data->swapchain = swapchain;
    data->window_size = ImVec2(instance_data->params.width, instance_data->params.height);
    data->font_atlas = IM_NEW(ImFontAtlas);
    map_object(HKEY(data->swapchain), data);
    return data;
}

static VkResult overlay_CreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkSwapchainKHR *pSwapchain) {

    VkSwapchainCreateInfoKHR createInfo = *pCreateInfo;

    createInfo.imageUsage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    struct device_data *device_data = FIND(struct device_data, device);
    //array<VkPresentModeKHR, 4> modes = {VK_PRESENT_MODE_FIFO_RELAXED_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_FIFO_KHR};

    VkResult result = device_data->vtable.CreateSwapchainKHR(device, &createInfo, pAllocator, pSwapchain);
    if (result != VK_SUCCESS)
        return result;

    struct swapchain_data *swapchain_data = new_swapchain_data(*pSwapchain, device_data);

    setup_swapchain_data(swapchain_data, pCreateInfo);

    const VkPhysicalDeviceProperties &prop = device_data->properties;
    swapchain_data->sw_stats.version_vk.major = VK_VERSION_MAJOR(prop.apiVersion);
    swapchain_data->sw_stats.version_vk.minor = VK_VERSION_MINOR(prop.apiVersion);
    swapchain_data->sw_stats.version_vk.patch = VK_VERSION_PATCH(prop.apiVersion);
    swapchain_data->sw_stats.engineName = device_data->instance->engineName;
    swapchain_data->sw_stats.engineVersion = device_data->instance->engineVersion;

    return result;
}

static VkResult overlay_CreateInstance(const VkInstanceCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkInstance *pInstance) {
    VkLayerInstanceCreateInfo *chain_info = get_instance_chain_info(pCreateInfo, VK_LAYER_LINK_INFO);

    assert(chain_info->u.pLayerInfo);
    PFN_vkGetInstanceProcAddr fpGetInstanceProcAddr = chain_info->u.pLayerInfo->pfnNextGetInstanceProcAddr;
    PFN_vkCreateInstance fpCreateInstance = (PFN_vkCreateInstance)fpGetInstanceProcAddr(NULL, "vkCreateInstance");
    if (fpCreateInstance == NULL) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    chain_info->u.pLayerInfo = chain_info->u.pLayerInfo->pNext;

    VkResult result = fpCreateInstance(pCreateInfo, pAllocator, pInstance);
    if (result != VK_SUCCESS)
        return result;

    struct instance_data *instance_data = new_instance_data(*pInstance);
    vk_load_instance_commands(instance_data->instance, fpGetInstanceProcAddr, &instance_data->vtable);
    instance_data_map_physical_devices(instance_data, true);

    return result;
}

static struct queue_data *new_queue_data(VkQueue queue, const VkQueueFamilyProperties *family_props, uint32_t family_index, struct device_data *device_data) {
    struct queue_data *data = new queue_data();
    data->device = device_data;
    data->queue = queue;
    data->flags = family_props->queueFlags;
    data->family_index = family_index;
    map_object(HKEY(data->queue), data);

    if (data->flags & VK_QUEUE_GRAPHICS_BIT)
        device_data->graphic_queue = data;

    return data;
}

static void device_map_queues(struct device_data *data, const VkDeviceCreateInfo *pCreateInfo) {
    uint32_t n_queues = 0;
    for (uint32_t i = 0; i < pCreateInfo->queueCreateInfoCount; i++)
        n_queues += pCreateInfo->pQueueCreateInfos[i].queueCount;
    data->queues.resize(n_queues);

    struct instance_data *instance_data = data->instance;
    uint32_t n_family_props;
    instance_data->vtable.GetPhysicalDeviceQueueFamilyProperties(data->physical_device, &n_family_props, NULL);
    std::vector<VkQueueFamilyProperties> family_props(n_family_props);
    instance_data->vtable.GetPhysicalDeviceQueueFamilyProperties(data->physical_device, &n_family_props, family_props.data());

    uint32_t queue_index = 0;
    for (uint32_t i = 0; i < pCreateInfo->queueCreateInfoCount; i++) {
        for (uint32_t j = 0; j < pCreateInfo->pQueueCreateInfos[i].queueCount; j++) {
            VkQueue queue;
            data->vtable.GetDeviceQueue(data->device, pCreateInfo->pQueueCreateInfos[i].queueFamilyIndex, j, &queue);

            VK_CHECK(data->set_device_loader_data(data->device, queue));

            data->queues[queue_index++] = new_queue_data(queue, &family_props[pCreateInfo->pQueueCreateInfos[i].queueFamilyIndex], pCreateInfo->pQueueCreateInfos[i].queueFamilyIndex, data);
        }
    }
}

static bool first_frame_ran = false;
static void compute_swapchain_display(struct swapchain_data *data) {
    if (!first_frame_ran) {
        onFirstFrame();
        first_frame_ran = true;
    }

    ImGui::SetCurrentContext(data->imgui_context);

    ImGui::NewFrame();

    onFrame(data->window_size, true);

    ImGui::EndFrame();
    ImGui::Render();
}

static struct overlay_draw *before_present(struct swapchain_data *swapchain_data, struct queue_data *present_queue, const VkSemaphore *wait_semaphores, unsigned n_wait_semaphores, unsigned imageIndex) {
    struct overlay_draw *draw = NULL;

    if (/*swapchain_data->sw_stats.n_frames > 0*/ true) {
        compute_swapchain_display(swapchain_data);
        draw = render_swapchain_display(swapchain_data, present_queue, wait_semaphores, n_wait_semaphores, imageIndex);
    }

    return draw;
}

static VkLayerDeviceCreateInfo *get_device_chain_info(const VkDeviceCreateInfo *pCreateInfo, VkLayerFunction func) {
    vk_foreach_struct(item, pCreateInfo->pNext) {
        if (item->sType == VK_STRUCTURE_TYPE_LOADER_DEVICE_CREATE_INFO && ((VkLayerDeviceCreateInfo *)item)->function == func)
            return (VkLayerDeviceCreateInfo *)item;
    }
    return NULL;
}

static VkResult overlay_CreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDevice *pDevice) {
    struct instance_data *instance_data = FIND(struct instance_data, physicalDevice);
    VkLayerDeviceCreateInfo *chain_info = get_device_chain_info(pCreateInfo, VK_LAYER_LINK_INFO);

    assert(chain_info->u.pLayerInfo);
    PFN_vkGetInstanceProcAddr fpGetInstanceProcAddr = chain_info->u.pLayerInfo->pfnNextGetInstanceProcAddr;
    PFN_vkGetDeviceProcAddr fpGetDeviceProcAddr = chain_info->u.pLayerInfo->pfnNextGetDeviceProcAddr;
    PFN_vkCreateDevice fpCreateDevice = (PFN_vkCreateDevice)fpGetInstanceProcAddr(NULL, "vkCreateDevice");
    if (fpCreateDevice == NULL) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    // Advance the link info for the next element on the chain
    chain_info->u.pLayerInfo = chain_info->u.pLayerInfo->pNext;

    std::vector<const char *> enabled_extensions(pCreateInfo->ppEnabledExtensionNames, pCreateInfo->ppEnabledExtensionNames + pCreateInfo->enabledExtensionCount);

    uint32_t extension_count;
    instance_data->vtable.EnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extension_count, nullptr);

    std::vector<VkExtensionProperties> available_extensions(extension_count);
    instance_data->vtable.EnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extension_count, available_extensions.data());

    bool can_get_driver_info = instance_data->api_version < VK_API_VERSION_1_1 ? false : true;

    // VK_KHR_driver_properties became core in 1.2
    if (instance_data->api_version < VK_API_VERSION_1_2 && can_get_driver_info) {
        for (auto &extension : available_extensions) {
            if (extension.extensionName == std::string(VK_KHR_DRIVER_PROPERTIES_EXTENSION_NAME)) {
                for (auto &enabled : enabled_extensions) {
                    if (enabled == std::string(VK_KHR_DRIVER_PROPERTIES_EXTENSION_NAME))
                        goto DONT;
                }
                enabled_extensions.push_back(VK_KHR_DRIVER_PROPERTIES_EXTENSION_NAME);
            DONT:
                goto FOUND;
            }
        }
        can_get_driver_info = false;
    FOUND:;
    }

    VkResult result = fpCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);
    if (result != VK_SUCCESS)
        return result;

    struct device_data *device_data = new_device_data(*pDevice, instance_data);
    device_data->physical_device = physicalDevice;
    vk_load_device_commands(*pDevice, fpGetDeviceProcAddr, &device_data->vtable);

    instance_data->vtable.GetPhysicalDeviceProperties(device_data->physical_device, &device_data->properties);

    VkLayerDeviceCreateInfo *load_data_info = get_device_chain_info(pCreateInfo, VK_LOADER_DATA_CALLBACK);
    device_data->set_device_loader_data = load_data_info->u.pfnSetDeviceLoaderData;

    driverProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES;
    driverProps.pNext = nullptr;
    if (can_get_driver_info) {
        VkPhysicalDeviceProperties2 deviceProps = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2, &driverProps};
        instance_data->vtable.GetPhysicalDeviceProperties2(device_data->physical_device, &deviceProps);
    }

    device_map_queues(device_data, pCreateInfo);

    return result;
}

static VkResult overlay_QueuePresentKHR(VkQueue queue, const VkPresentInfoKHR *pPresentInfo) {
    using namespace std::chrono_literals;

    struct queue_data *queue_data = FIND(struct queue_data, queue);

    /* Otherwise we need to add our overlay drawing semaphore to the list of
     * semaphores to wait on. If we don't do that the presented picture might
     * be have incomplete overlay drawings.
     */
    VkResult result = VK_SUCCESS;
    for (uint32_t i = 0; i < pPresentInfo->swapchainCount; i++) {
        VkSwapchainKHR swapchain = pPresentInfo->pSwapchains[i];
        struct swapchain_data *swapchain_data = FIND(struct swapchain_data, swapchain);

        uint32_t image_index = pPresentInfo->pImageIndices[i];

        VkPresentInfoKHR present_info = *pPresentInfo;
        present_info.swapchainCount = 1;
        present_info.pSwapchains = &swapchain;
        present_info.pImageIndices = &image_index;

        struct overlay_draw *draw = before_present(swapchain_data, queue_data, pPresentInfo->pWaitSemaphores, i == 0 ? pPresentInfo->waitSemaphoreCount : 0, image_index);

        if (draw) {
            present_info.pWaitSemaphores = &draw->semaphore;
            present_info.waitSemaphoreCount = 1;
        }

        VkResult chain_result = queue_data->device->vtable.QueuePresentKHR(queue, &present_info);
        if (pPresentInfo->pResults)
            pPresentInfo->pResults[i] = chain_result;
        if (chain_result != VK_SUCCESS && result == VK_SUCCESS)
            result = chain_result;
    }

    return result;
}

static VkResult overlay_AllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo *pAllocateInfo, VkCommandBuffer *pCommandBuffers) {
    struct device_data *device_data = FIND(struct device_data, device);
    VkResult result = device_data->vtable.AllocateCommandBuffers(device, pAllocateInfo, pCommandBuffers);
    if (result != VK_SUCCESS)
        return result;

    for (uint32_t i = 0; i < pAllocateInfo->commandBufferCount; i++) {
        new_command_buffer_data(pCommandBuffers[i], pAllocateInfo->level, device_data);
    }

    return result;
}

extern "C" VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL overlay_GetInstanceProcAddr(VkInstance instance, const char *funcName);
extern "C" VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL overlay_GetDeviceProcAddr(VkDevice device, const char *funcName);

std::vector<std::pair<std::string, void *>> name_to_funcptr_map{{"vkGetInstanceProcAddr", (void *)overlay_GetInstanceProcAddr},
                                                                {"vkGetDeviceProcAddr", (void *)overlay_GetDeviceProcAddr},
                                                                {"vkCreateInstance", (void *)overlay_CreateInstance},
                                                                {"vkQueuePresentKHR", (void *)overlay_QueuePresentKHR},
                                                                {"vkCreateSwapchainKHR", (void *)overlay_CreateSwapchainKHR},
                                                                {"vkCreateDevice", (void *)overlay_CreateDevice},
                                                                {"vkAllocateCommandBuffers", (void *)overlay_AllocateCommandBuffers}};

static void *find_ptr(const char *name) {
    for (uint32_t i = 0; i < name_to_funcptr_map.size(); i++) {
        if (strcmp(name, name_to_funcptr_map[i].first.c_str()) == 0) {
            return name_to_funcptr_map[i].second;
        }
    }

    return NULL;
}

extern "C" VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL overlay_GetInstanceProcAddr(VkInstance instance, const char *funcName) {
    void *ptr = find_ptr(funcName);
    if (ptr)
        return reinterpret_cast<PFN_vkVoidFunction>(ptr);
    if (instance == NULL)
        return NULL;

    struct instance_data *instance_data = FIND(struct instance_data, instance);
    if (instance_data->vtable.GetInstanceProcAddr == NULL)
        return NULL;
    return instance_data->vtable.GetInstanceProcAddr(instance, funcName);
}

extern "C" VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL overlay_GetDeviceProcAddr(VkDevice dev, const char *funcName) {
    void *ptr = find_ptr(funcName);

    std::cout << "overlay_GetDeviceProcAddr: " << funcName << std::endl;

    if (ptr)
        return reinterpret_cast<PFN_vkVoidFunction>(ptr);
    
    if (dev == NULL)
        return NULL;

    struct device_data *device_data = FIND(struct device_data, dev);
    if (device_data->vtable.GetDeviceProcAddr == NULL)
        return NULL;
    return device_data->vtable.GetDeviceProcAddr(dev, funcName);
}