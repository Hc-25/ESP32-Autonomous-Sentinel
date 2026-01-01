#include "detect.hpp"
#include <filesystem>

#if CONFIG_DETECT_MODEL_IN_FLASH_RODATA
extern const uint8_t detect_espdl[] asm("_binary_detect_espdl_start");
static const char *path = (const char *)detect_espdl;
#elif CONFIG_DETECT_MODEL_IN_FLASH_PARTITION
static const char *path = "detect";
#else
#if !defined(CONFIG_BSP_SD_MOUNT_POINT)
#define CONFIG_BSP_SD_MOUNT_POINT "/sdcard"
#endif
#endif
namespace detect {
Pico::Pico(const char *model_name, float score_thr, float nms_thr)
{
#if !CONFIG_DETECT_MODEL_IN_SDCARD
    m_model = new dl::Model(
        path, model_name, static_cast<fbs::model_location_type_t>(CONFIG_DETECT_MODEL_LOCATION));
#else
    auto sd_path =
        std::filesystem::path(CONFIG_BSP_SD_MOUNT_POINT) / CONFIG_DETECT_MODEL_SDCARD_DIR / model_name;
    m_model = new dl::Model(sd_path.c_str(), fbs::MODEL_LOCATION_IN_SDCARD);
#endif
    m_model->minimize();
#if CONFIG_IDF_TARGET_ESP32P4
    m_image_preprocessor = new dl::image::ImagePreprocessor(m_model, {0, 0, 0}, {1, 1, 1});
#else
    m_image_preprocessor =
        new dl::image::ImagePreprocessor(m_model, {0, 0, 0}, {1, 1, 1}, dl::image::DL_IMAGE_CAP_RGB565_BIG_ENDIAN);
#endif
    m_postprocessor = new dl::detect::PicoPostprocessor(
        m_model, m_image_preprocessor, score_thr, nms_thr, 10, {{8, 8, 4, 4}, {16, 16, 8, 8}, {32, 32, 16, 16}});
}
} // namespace detect

Detect::Detect(model_type_t model_type, bool lazy_load) : m_model_type(model_type)
{
    switch (model_type) {
    case model_type_t::PICO_S8_V1:
        m_score_thr[0] = detect::Pico::default_score_thr;
        m_nms_thr[0] = detect::Pico::default_nms_thr;
        break;
    }
    if (lazy_load) {
        m_model = nullptr;
    } else {
        load_model();
    }
}

void Detect::load_model()
{
    switch (m_model_type) {
    case model_type_t::PICO_S8_V1:
#if CONFIG_FLASH_DETECT_PICO_S8_V1 || CONFIG_DETECT_MODEL_IN_SDCARD
        m_model = new detect::Pico("detect_pico_s8_v1.espdl", m_score_thr[0], m_nms_thr[0]);
#else
        ESP_LOGE("detect", "detect_pico_s8_v1 is not selected in menuconfig.");
#endif
        break;
    }
}
