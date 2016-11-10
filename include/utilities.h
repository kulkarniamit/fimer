#ifndef _UTILITIES_H_
#define _UTILITIES_H_

/* MACRO FUNCTIONS  */
#define MINUTES_TO_SECONDS(m) (m * 60)
#define HOURS_TO_SECONDS(h) (h * MINUTES_TO_SECONDS(60))
#define DAYS_TO_SECONDS(d) (d * HOURS_TO_SECONDS(24))
#define WEEKS_TO_SECONDS(w) (w * DAYS_TO_SECONDS(7))
#define MONTHS_TO_SECONDS(months) (months * WEEKS_TO_SECONDS(4))
#define YEARS_TO_SECONDS(y) (y * MONTHS_TO_SECONDS(12))

#endif