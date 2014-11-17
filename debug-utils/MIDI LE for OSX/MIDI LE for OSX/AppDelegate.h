//
//  AppDelegate.h
//  MIDI LE for OSX
//
//  Created by Matthias Frick on 08.10.2014.
//  Copyright (c) 2014 Matthias Frick. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <IOBluetooth/IOBluetooth.h>
#import "PGMidi.h"
#import "NSData+hexConv.h"

@interface AppDelegate : NSObject <NSApplicationDelegate, CBCentralManagerDelegate, CBPeripheralDelegate, PGMidiDelegate, PGMidiSourceDelegate>
{

    
    NSTimer *pulseTimer;
  //  NSArrayController *arrayController;
    
    CBCentralManager *manager;
    CBPeripheral *peripheral;
    
    
    
    NSString *manufacturer;
    
    uint16_t heartRate;
    
    IBOutlet NSButton* connectButton;
    BOOL autoConnect;
    
    // Progress Indicator
    IBOutlet NSButton * indicatorButton;
    IBOutlet NSProgressIndicator *progressIndicator;
}


@property (assign) IBOutlet NSWindow *window;
@property (assign) IBOutlet NSWindow *scanSheet;
@property (assign) IBOutlet NSView *bleView;
@property (assign) IBOutlet NSArrayController *arrayController;
@property (nonatomic, strong) IBOutlet NSTextView *debugView;
@property (assign) uint16_t heartRate;
@property (retain) NSTimer *pulseTimer;
@property (nonatomic, strong) IBOutlet NSMutableArray *heartRateMonitors;
@property (copy) NSString *manufacturer;
@property (copy) NSString *connected;

- (IBAction) openScanSheet:(id) sender;
- (IBAction) closeScanSheet:(id)sender;
- (IBAction) cancelScanSheet:(id)sender;
- (IBAction) connectButtonPressed:(id)sender;

- (void) startScan;
- (void) stopScan;
- (BOOL) isLECapableHardware;

// - (void) pulse;
- (void) updateWithHRMData:(NSData *)data;





@end

