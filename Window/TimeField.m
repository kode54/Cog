//
//  TimeField.m
//  Cog
//
//  Created by Vincent Spader on 2/22/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import "TimeField.h"

static NSString *kTimerModeKey = @"timerShowTimeRemaining";

NSString * timeStringForTimeInterval(NSTimeInterval timeInterval) {
	const int64_t signed_total_seconds = (int64_t)timeInterval;
	const bool need_minus_sign = signbit(timeInterval);
	const int64_t total_seconds = (need_minus_sign ? -1 : 1) * signed_total_seconds;
	const int64_t seconds = total_seconds % 60;
	const int64_t total_minutes = (total_seconds - seconds) / 60;
	const int64_t minutes = total_minutes % 60;
	const int64_t total_hours = (total_minutes - minutes) / 60;
	const int64_t hours = total_hours % 24;
	const int64_t days = (total_hours - hours) / 24;
	
	NSString *timeString = nil;
	
	if (days > 0) {
		timeString =
		[NSString localizedStringWithFormat:@"%s" "%" PRIi64 ":" "%02" PRIi64 ":" "%02" PRIi64 ":" "%02" PRIi64,
		 need_minus_sign ? "-" : "",
		 days,
		 hours,
		 minutes,
		 seconds];
	}
	else if (hours > 0) {
		timeString =
		[NSString localizedStringWithFormat:@"%s" "%" PRIi64 ":" "%02" PRIi64 ":" "%02" PRIi64,
		 need_minus_sign ? "-" : "",
		 hours,
		 minutes,
		 seconds];
	}
	else if (minutes > 0) {
		timeString =
		[NSString localizedStringWithFormat:@"%s" "%" PRIi64 ":" "%02" PRIi64,
		 need_minus_sign ? "-" : "",
		 minutes,
		 seconds];
	}
	else {
		timeString =
		[NSString localizedStringWithFormat:@"%s" "0" ":" "%02" PRIi64,
		 need_minus_sign ? "-" : "",
		 seconds];
	}

	return timeString;
}


@implementation TimeField {
    BOOL showTimeRemaining;
    NSDictionary *fontAttributes;
}

- (void)awakeFromNib
{
    showTimeRemaining = [[NSUserDefaults standardUserDefaults] boolForKey:kTimerModeKey];
}

- (void)update
{
    NSString *text;
    if (showTimeRemaining == NO)
    {
		NSTimeInterval sec = self.currentTime;
        text = timeStringForTimeInterval(sec);
    }
    else
    {
		NSTimeInterval sec = self.currentTime - self.duration;
		// NOTE: The floating point standard has support for negative zero.
		// We use that to enforce the sign prefix.
		if (sec == 0.0) { sec = -0.0; }
		text = timeStringForTimeInterval(sec);
    }
    NSAttributedString *string = [[NSAttributedString alloc] initWithString:text
                                                                 attributes:fontAttributes];
    [self setAttributedStringValue: string];
}

- (void)mouseDown:(NSEvent *)theEvent
{
    showTimeRemaining = !showTimeRemaining;
    [[NSUserDefaults standardUserDefaults] setBool:showTimeRemaining forKey:kTimerModeKey];
    [self update];
}

- (void)setCurrentTime:(NSTimeInterval)currentTime
{
    _currentTime = currentTime;
    [self update];
}

@end
