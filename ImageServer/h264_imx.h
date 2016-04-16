//
// Created by ghdawn on 16-1-20.
//

#ifndef IMAGESERVER_H264_IMX_H
#define IMAGESERVER_H264_IMX_H
#include "itrbase.h"
#include "imxvpuapi/imxvpuapi.h"

typedef struct _Context
{
    ImxVpuEncoder *vpuenc;

    ImxVpuDMABuffer *bitstream_buffer;
    size_t bitstream_buffer_size;
    unsigned int bitstream_buffer_alignment;

    ImxVpuEncInitialInfo initial_info;

    ImxVpuFramebuffer input_framebuffer;
    ImxVpuDMABuffer *input_fb_dmabuffer;

    ImxVpuFramebuffer *framebuffers;
    ImxVpuDMABuffer **fb_dmabuffers;
    unsigned int num_framebuffers;
    ImxVpuFramebufferSizes calculated_sizes;
}Context;


class H264_imx
{
public:
    H264_imx();

    void Open(int width, int height, int fps);

    void Compress(U8 *data, U8 *compressed, int &length);
    void SetQuality(int quality);
    void Close();
private:
    void run(Context *ctx, U8 *data, U8 *compressed, int &length);
    Context *ctx;
    int framesize;
    int quality;
};


#endif //IMAGESERVER_H264_IMX_H
