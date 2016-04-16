//
// Created by ghdawn on 16-1-20.
//

#include "h264_imx.h"
#include "stdlib.h"
#include "string.h"

void* acquire_output_buffer(void *context, size_t size, void **acquired_handle)
{
    void *mem;

    ((void) (context));

    /* In this example, "acquire" a buffer by simply allocating it with malloc() */
    mem = malloc(size);
    *acquired_handle = mem;
    return mem;
}

void finish_output_buffer(void *context, void *acquired_handle)
{
    ((void) (context));

    /* Nothing needs to be done here in this example. Just log this call. */
}

void shutdown(Context *ctx)
{
    unsigned int i;

    /* Close the previously opened encoder instance */
    imx_vpu_enc_close(ctx->vpuenc);

    /* Free all allocated memory (both regular and DMA memory) */
    imx_vpu_dma_buffer_deallocate(ctx->input_fb_dmabuffer);
    free(ctx->framebuffers);
    for (i = 0; i < ctx->num_framebuffers; ++i)
        imx_vpu_dma_buffer_deallocate(ctx->fb_dmabuffers[i]);
    free(ctx->fb_dmabuffers);
    imx_vpu_dma_buffer_deallocate(ctx->bitstream_buffer);

    /* Unload the VPU firmware */
    imx_vpu_enc_unload();

    free(ctx);
}

ImxVpuEncodedFrame output_frame;
Context* init(int width, int height, int fps)
{
    Context *ctx;
    ImxVpuEncOpenParams open_params;
    unsigned int i;

    ctx = (Context*)calloc(1, sizeof(Context));

    /* Set the open params. Use the default values (note that memset must still
     * be called to ensure all values are set to 0 initially; the
     * imx_vpu_enc_set_default_open_params() function does not do this!).
     * Then, set a bitrate of 0 kbps, which tells the VPU to use constant quality
     * mode instead (controlled by the quant_param field in ImxVpuEncParams).
     * Frame width & height are also necessary, as are the frame rate numerator
     * and denominator. */
    memset(&open_params, 0, sizeof(open_params));
    imx_vpu_enc_set_default_open_params(IMX_VPU_CODEC_FORMAT_H264, &open_params);
    open_params.bitrate = 4000;
    open_params.frame_width = width;
    open_params.frame_height = height;
    open_params.frame_rate_numerator = fps;
    open_params.frame_rate_denominator = 1;


    /* Set up the output frame. Simply setting all fields to zero/NULL
     * is enough. The encoder will fill in data. */
    memset(&output_frame, 0, sizeof(output_frame));

    /* Load the VPU firmware */
    imx_vpu_enc_load();

    /* Retrieve information about the required bitstream buffer and allocate one based on this */
    imx_vpu_enc_get_bitstream_buffer_info(&(ctx->bitstream_buffer_size), &(ctx->bitstream_buffer_alignment));
    ctx->bitstream_buffer = imx_vpu_dma_buffer_allocate(
            imx_vpu_enc_get_default_allocator(),
            ctx->bitstream_buffer_size,
            ctx->bitstream_buffer_alignment,
            0
    );

    /* Open an encoder instance, using the previously allocated bitstream buffer */
    imx_vpu_enc_open(&(ctx->vpuenc), &open_params, ctx->bitstream_buffer);


    /* Retrieve the initial information to allocate framebuffers for the
     * encoding process (unlike with decoding, these framebuffers are used
     * only internally by the encoder as temporary storage; encoded data
     * doesn't go in there, nor do raw input frames) */
    imx_vpu_enc_get_initial_info(ctx->vpuenc, &(ctx->initial_info));

    ctx->num_framebuffers = ctx->initial_info.min_num_required_framebuffers;
    fprintf(stderr, "num framebuffers: %u\n", ctx->num_framebuffers);

    /* Using the initial information, calculate appropriate framebuffer sizes */
    imx_vpu_calc_framebuffer_sizes(IMX_VPU_COLOR_FORMAT_YUV420, width, height,
                                   ctx->initial_info.framebuffer_alignment, 0, 0, &(ctx->calculated_sizes));
    fprintf(
            stderr,
            "calculated sizes:  frame width&height: %dx%d  Y stride: %u  CbCr stride: %u  Y size: %u  CbCr size: %u  MvCol size: %u  total size: %u\n",
            ctx->calculated_sizes.aligned_frame_width, ctx->calculated_sizes.aligned_frame_height,
            ctx->calculated_sizes.y_stride, ctx->calculated_sizes.cbcr_stride,
            ctx->calculated_sizes.y_size, ctx->calculated_sizes.cbcr_size, ctx->calculated_sizes.mvcol_size,
            ctx->calculated_sizes.total_size
    );


    /* Allocate memory blocks for the framebuffer and DMA buffer structures,
     * and allocate the DMA buffers themselves */

    ctx->framebuffers = malloc(sizeof(ImxVpuFramebuffer) * ctx->num_framebuffers);
    ctx->fb_dmabuffers = malloc(sizeof(ImxVpuDMABuffer* ) * ctx->num_framebuffers);

    for (i = 0; i < ctx->num_framebuffers; ++i)
    {
        /* Allocate a DMA buffer for each framebuffer. It is possible to specify alternate allocators;
         * all that is required is that the allocator provides physically contiguous memory
         * (necessary for DMA transfers) and respecs the alignment value. */
        ctx->fb_dmabuffers[i] = imx_vpu_dma_buffer_allocate(imx_vpu_dec_get_default_allocator(),
                                                            ctx->calculated_sizes.total_size,
                                                            ctx->initial_info.framebuffer_alignment, 0);

        imx_vpu_fill_framebuffer_params(&(ctx->framebuffers[i]), &(ctx->calculated_sizes), ctx->fb_dmabuffers[i],
                                        0);
    }

    /* allocate DMA buffers for the raw input frames. Since the encoder can only read
     * raw input pixels from a DMA memory region, it is necessary to allocate one,
     * and later copy the pixels into it. In production, it is generally a better
     * idea to make sure that the raw input frames are already placed in DMA memory
     * (either allocated by imx_vpu_dma_buffer_allocate() or by some other means of
     * getting DMA / physically contiguous memory with known physical addresses). */
    ctx->input_fb_dmabuffer = imx_vpu_dma_buffer_allocate(imx_vpu_dec_get_default_allocator(),
                                                          ctx->calculated_sizes.total_size,
                                                          ctx->initial_info.framebuffer_alignment, 0);
    imx_vpu_fill_framebuffer_params(&(ctx->input_framebuffer), &(ctx->calculated_sizes), ctx->input_fb_dmabuffer,
                                    0);

    /* Actual registration is done here. From this moment on, the VPU knows which buffers to use for
     * storing temporary frames into. This call must not be done again until encoding is shut down. */
    imx_vpu_enc_register_framebuffers(ctx->vpuenc, ctx->framebuffers, ctx->num_framebuffers);

    return ctx;
}

H264_imx::H264_imx() : quality(0)
{ }

void H264_imx::Open(int width, int height, int fps)
{
    ctx = init(width, height, fps);
    framesize = width * height * 3 / 2;
}

void H264_imx::Compress(U8 *data, U8 *compressed, int &length)
{
    run(ctx, data, compressed, length);
}

void H264_imx::SetQuality(int quality)
{
    this->quality = quality;
}

void H264_imx::run(Context *ctx, U8 *data, U8 *compressed, int &length)
{
    ImxVpuRawFrame input_frame;
    ImxVpuEncParams enc_params;
    unsigned int output_code;

    /* Set up the input frame. The only field that needs to be
     * set is the input framebuffer. The encoder will read from it.
     * The rest can remain zero/NULL. */
    memset(&input_frame, 0, sizeof(input_frame));
    input_frame.framebuffer = &(ctx->input_framebuffer);

    /* Set the encoding parameters for this frame. quant_param 0 is
     * the highest quality in h.264 constant quality encoding mode.
     * (The range in h.264 is 0-51, where 0 is best quality and worst
     * compression, and 51 vice versa.) */
    memset(&enc_params, 0, sizeof(enc_params));
    enc_params.force_I_frame = 0;
    enc_params.quant_param = quality;
    enc_params.acquire_output_buffer = acquire_output_buffer;
    enc_params.finish_output_buffer = finish_output_buffer;
    enc_params.output_buffer_context = NULL;


    /* Read input i420 frames and encode them until the end of the input file is reached */

    uint8_t *mapped_virtual_address;
    void *output_block;

    /* Read uncompressed pixels into the input DMA buffer */
    mapped_virtual_address = imx_vpu_dma_buffer_map(ctx->input_fb_dmabuffer, IMX_VPU_MAPPING_FLAG_WRITE);
    // fread(mapped_virtual_address, 1, framesize, ctx->fin);
    memcpy(mapped_virtual_address, data, framesize);
    imx_vpu_dma_buffer_unmap(ctx->input_fb_dmabuffer);


    /* The actual encoding */
    imx_vpu_enc_encode(ctx->vpuenc, &input_frame, &output_frame, &enc_params, &output_code);

    /* Write out the encoded frame to the output file. The encoder
     * will have called acquire_output_buffer(), which acquires a
     * buffer by malloc'ing it. The "handle" in this example is
     * just the pointer to the allocated memory. This means that
     * here, acquired_handle is the pointer to the encoded frame
     * data. Write it to file, and then free the previously
     * allocated block. In production, the acquire function could
     * retrieve an output memory block from a buffer pool for
     * example. */

    output_block = output_frame.acquired_handle;
    // fwrite(output_block, 1, output_frame.data_size,fp);//ctx->fout);
    length = output_frame.data_size;
    memcpy(compressed, output_block, length);
    free(output_block);
    printf("para: %d %d\n",enc_params.force_I_frame, enc_params.quant_param = quality);
    printf("para: %d\n",output_frame.frame_type);

}

void H264_imx::Close()
{
    shutdown(ctx);
}
