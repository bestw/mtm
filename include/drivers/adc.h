/**
 * @brief 
 * @author 
 * @date 
 *
 * update:
 */
#ifndef DRIVERS_ADC_H_
#define DRIVERS_ADC_H_

struct AdcDevice
{
    struct Device_t dev;
    int mode;
    struct 
    {
        uint8_t *buffer;
        int length;
    } ;
    int internal;
    int counts;
#define ADC_SOFT_FILTER_NONE 0
#define ADC_SOFT_FILTER_MEAN 1
    int soft_filter_type;
    int (*start_adc)(struct AdcDevice *adcdev, int flags);
    int (*stop_adc)(struct AdcDevice *adcdev);
    int (*get_data)(struct AdcDevice *advdev, uint8_t *buf, int size);
};

#endif /* DRIVERS_ADC_H_ */
/* vim:set et ts=4 sts=4 sw=4 ft=c: */
