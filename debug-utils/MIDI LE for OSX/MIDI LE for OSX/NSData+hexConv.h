//
//  NSData+hexConv.h
//  MIDI LE for OSX
//
//  Created by Matthias Frick on 08.10.2014.
//  Copyright (c) 2014 Matthias Frick. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface NSData (hexConv)
-(NSString*)hexRepresentationWithSpaces_AS:(BOOL)spaces;
@end
