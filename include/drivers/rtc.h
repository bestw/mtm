/**
 * @file rtc.h
 * @brief rtc device defines and others.
 * @author WANG Jun
 * @date 2017-09-01
 *
 * update:
 */
#ifndef DRIVERS_RTC_H_
#define DRIVERS_RTC_H_

int rtcGetTime(time_t *tm);
int rtcSetTime(time_t *tm);

#endif /* DRIVERS_RTC_H_ */