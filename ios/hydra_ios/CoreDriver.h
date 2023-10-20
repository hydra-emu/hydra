#ifndef CORE_DRIVER_H
#define CORE_DRIVER_H
#import <Foundation/Foundation.h>

@interface CoreDriver : NSObject {
    void* cppImplementation;
}

+ (BOOL) create:(NSString*)corePath;
+ (void) resetCore:();
+ (void) runFrame:();
+ (void) destroyCore:();

@end

#endif
