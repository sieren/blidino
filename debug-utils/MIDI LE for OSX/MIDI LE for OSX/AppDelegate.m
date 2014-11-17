//
//  AppDelegate.m
//  MIDI LE for OSX
//
//  Created by Matthias Frick on 08.10.2014.
//  Copyright (c) 2014 Matthias Frick. All rights reserved.
//

#import "AppDelegate.h"
#import <QuartzCore/QuartzCore.h>
#import <CoreMIDI/CoreMIDI.h>
OSStatus s;
MIDIClientRef client;
MIDIPortRef outputPort;
char pktBuffer[1024];
MIDIPacketList* pktList = (MIDIPacketList*) pktBuffer;
MIDIPacket     *pkt;
MIDIEndpointRef midiOut;
@interface AppDelegate () {

}
@property (nonatomic,strong) PGMidi *midi;
@property (nonatomic, strong) NSMutableArray *sources;
@property (nonatomic, strong) NSMutableArray *destinations;

@end


@implementation AppDelegate
@synthesize window;
@synthesize heartRate;
@synthesize bleView;
@synthesize pulseTimer;
@synthesize destinations;
@synthesize scanSheet;
@synthesize heartRateMonitors;
@synthesize arrayController;
@synthesize manufacturer;
@synthesize midi;
@synthesize sources;
@synthesize connected;
@synthesize  debugView;
- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    // Insert code here to initialize your application
    
    self.heartRate = 0;
    autoConnect = FALSE;   /* uncomment this line if you want to automatically connect to previosly known peripheral */
    self.heartRateMonitors = [NSMutableArray array];
    
    [NSAnimationContext beginGrouping];
    [[NSAnimationContext currentContext] setDuration:0.];
 //   [self.heartView layer].position = CGPointMake( [[self.heartView layer] frame].size.width / 2, [[self.heartView layer] frame].size.height / 2 );
//    [self.heartView layer].anchorPoint = CGPointMake(0.5, 0.5);
    [NSAnimationContext endGrouping];
    
    manager = [[CBCentralManager alloc] initWithDelegate:self queue:nil];
    if( autoConnect )
    {
        [self startScan];
    }
    NSLog(@"Launched");
    self.midi = [[PGMidi alloc] init];
   self.sources = midi.sources;
    self.destinations = midi.destinations;
   // int             i;
  //  s = MIDIClientCreate((CFStringRef)@"My Test MIDI Client", MyMIDINotifyProc, (__bridge void *)(self), &client);
  //  s = MIDIOutputPortCreate(client, (CFStringRef)@"My Test MIDI Output Port", &outputPort);
    MIDIClientCreate(CFSTR("MIDI LE MIDI"), NULL, NULL,
                     &client);
    MIDISourceCreate(client, CFSTR("MIDI LE  Source"),
                     &midiOut);
    MIDIOutputPortCreate(client, CFSTR("MIDI LE Out Port"),
                         &outputPort);
}



/*
 Disconnect peripheral when application terminate
 */
- (void) applicationWillTerminate:(NSNotification *)notification
{
    if(peripheral)
    {
        [manager cancelPeripheralConnection:peripheral];
    }
}

#pragma mark - Scan sheet methods

/*
 Open scan sheet to discover heart rate peripherals if it is LE capable hardware
 */
- (IBAction)openScanSheet:(id)sender
{
    if( [self isLECapableHardware] )
    {
        autoConnect = FALSE;
        [arrayController removeObjects:self.heartRateMonitors];
        [NSApp beginSheet:self.scanSheet modalForWindow:self.window modalDelegate:self didEndSelector:@selector(sheetDidEnd:returnCode:contextInfo:) contextInfo:nil];
        [self startScan];
    }
}

/*
 Close scan sheet once device is selected
 */
- (IBAction)closeScanSheet:(id)sender
{
    [NSApp endSheet:self.scanSheet returnCode:NSAlertDefaultReturn];
    [self.scanSheet orderOut:self];
}

/*
 Close scan sheet without choosing any device
 */
- (IBAction)cancelScanSheet:(id)sender
{
    [NSApp endSheet:self.scanSheet returnCode:NSAlertAlternateReturn];
    [self.scanSheet orderOut:self];
}

/*
 This method is called when Scan sheet is closed. Initiate connection to selected heart rate peripheral
 */
- (void)sheetDidEnd:(NSWindow *)sheet returnCode:(NSInteger)returnCode contextInfo:(void *)contextInfo
{
    [self stopScan];
    if( returnCode == NSAlertDefaultReturn )
    {
        NSIndexSet *indexes = [self.arrayController selectionIndexes];
        if ([indexes count] != 0)
        {
            NSUInteger anIndex = [indexes firstIndex];
            peripheral = [self.heartRateMonitors objectAtIndex:anIndex];
            [indicatorButton setHidden:FALSE];
            [progressIndicator setHidden:FALSE];
            [progressIndicator startAnimation:self];
            [connectButton setTitle:@"Cancel"];
            [manager connectPeripheral:peripheral options:nil];
        }
    }
}

#pragma mark - Connect Button

/*
 This method is called when connect button pressed and it takes appropriate actions depending on device connection state
 */
- (IBAction)connectButtonPressed:(id)sender
{
    if(peripheral && ([peripheral isConnected]))
    {
        /* Disconnect if it's already connected */
        [manager cancelPeripheralConnection:peripheral];
    }
    else if (peripheral)
    {
        /* Device is not connected, cancel pendig connection */
        [indicatorButton setHidden:TRUE];
        [progressIndicator setHidden:TRUE];
        [progressIndicator stopAnimation:self];
        [connectButton setTitle:@"Connect"];
        [manager cancelPeripheralConnection:peripheral];
        [self openScanSheet:nil];
    }
    else
    {   /* No outstanding connection, open scan sheet */
        [self openScanSheet:nil];
    }
}

#pragma mark - Heart Rate Data


- (void) midiSource:(PGMidiSource*)midisrc midiReceived:(const MIDIPacketList *)packetList
{
    NSLog(@"Incoming from: %@", midisrc.name);
    
    [self performSelectorOnMainThread:@selector(addString:)
                           withObject:@"MIDI received:"
                        waitUntilDone:NO];
    
    // const MIDIPacket *packet = &packetList->packet[0];

    
    
    // NEW CODE
        for (PGMidiDestination *dest in midi.destinations) {
                const MIDIPacket *packet = &packetList->packet[0];
                for (int i = 0; i < packetList->numPackets; ++i)
                {
                   // [dest sendBytes:packet->data size:packet->length];
                 //   packet = MIDIPacketNext(packet);
                }
      
        
    }
    
}


#pragma mark - Start/Stop Scan methods

/*
 Uses CBCentralManager to check whether the current platform/hardware supports Bluetooth LE. An alert is raised if Bluetooth LE is not enabled or is not supported.
 */
- (BOOL) isLECapableHardware
{
    NSString * state = nil;
    
    switch ([manager state])
    {
        case CBCentralManagerStateUnsupported:
            state = @"The platform/hardware doesn't support Bluetooth Low Energy.";
            break;
        case CBCentralManagerStateUnauthorized:
            state = @"The app is not authorized to use Bluetooth Low Energy.";
            break;
        case CBCentralManagerStatePoweredOff:
            state = @"Bluetooth is currently powered off.";
            break;
        case CBCentralManagerStatePoweredOn:
            return TRUE;
        case CBCentralManagerStateUnknown:
        default:
            return FALSE;
            
    }
    
    NSLog(@"Central manager state: %@", state);
    
    [self cancelScanSheet:nil];
    
    NSAlert *alert = [[NSAlert alloc] init];
    [alert setMessageText:state];
    [alert addButtonWithTitle:@"OK"];
    [alert setIcon:[[NSImage alloc] initWithContentsOfFile:@"AppIcon"]];
    [alert beginSheetModalForWindow:[self window] modalDelegate:self didEndSelector:nil contextInfo:nil];
    return FALSE;
}

/*
 Request CBCentralManager to scan for heart rate peripherals using service UUID 0x180D
 */
- (void) startScan
{
    [manager scanForPeripheralsWithServices:[NSArray arrayWithObject:[CBUUID UUIDWithString:@"03B80E5A-EDE8-4B33-A751-6CE34EC4C700"]] options:nil];
}

/*
 Request CBCentralManager to stop scanning for heart rate peripherals
 */
- (void) stopScan
{
    [manager stopScan];
}

#pragma mark - CBCentralManager delegate methods
/*
 Invoked whenever the central manager's state is updated.
 */
- (void) centralManagerDidUpdateState:(CBCentralManager *)central
{
    [self isLECapableHardware];
}

/*
 Invoked when the central discovers heart rate peripheral while scanning.
 */
- (void) centralManager:(CBCentralManager *)central didDiscoverPeripheral:(CBPeripheral *)aPeripheral advertisementData:(NSDictionary *)advertisementData RSSI:(NSNumber *)RSSI
{
    NSLog(@"Device: %@",aPeripheral.UUID);
    NSMutableArray *peripherals = [self mutableArrayValueForKey:@"heartRateMonitors"];
    if( ![self.heartRateMonitors containsObject:aPeripheral] )
        [peripherals addObject:aPeripheral];

    /* Retreive already known devices */
    if(autoConnect)
    {
        [manager retrievePeripherals:[NSArray arrayWithObject:(id)aPeripheral.UUID]];
    }
}

/*
 Invoked when the central manager retrieves the list of known peripherals.
 Automatically connect to first known peripheral
 */
- (void)centralManager:(CBCentralManager *)central didRetrievePeripherals:(NSArray *)peripherals
{
    NSLog(@"Retrieved peripheral: %lu - %@", [peripherals count], peripherals);
    
    [self stopScan];
    
    /* If there are any known devices, automatically connect to it.*/
    if([peripherals count] >=1)
    {
        [indicatorButton setHidden:FALSE];
        [progressIndicator setHidden:FALSE];
        [progressIndicator startAnimation:self];
        peripheral = [peripherals objectAtIndex:0];
        [connectButton setTitle:@"Cancel"];
        [manager connectPeripheral:peripheral options:[NSDictionary dictionaryWithObject:[NSNumber numberWithBool:YES] forKey:CBConnectPeripheralOptionNotifyOnDisconnectionKey]];
    }
}

/*
 Invoked whenever a connection is succesfully created with the peripheral.
 Discover available services on the peripheral
 */
- (void) centralManager:(CBCentralManager *)central didConnectPeripheral:(CBPeripheral *)aPeripheral
{
    [aPeripheral setDelegate:self];
    [aPeripheral discoverServices:nil];
    
    self.connected = @"Connected";
    [connectButton setTitle:@"Disconnect"];
    [indicatorButton setHidden:TRUE];
    [progressIndicator setHidden:TRUE];
    [progressIndicator stopAnimation:self];
}

/*
 Invoked whenever an existing connection with the peripheral is torn down.
 Reset local variables
 */
- (void)centralManager:(CBCentralManager *)central didDisconnectPeripheral:(CBPeripheral *)aPeripheral error:(NSError *)error
{
    self.connected = @"Not connected";
    [connectButton setTitle:@"Connect"];
    self.manufacturer = @"";
    self.heartRate = 0;
    if( peripheral )
    {
        [peripheral setDelegate:nil];
        peripheral = nil;
    }
}

/*
 Invoked whenever the central manager fails to create a connection with the peripheral.
 */
- (void)centralManager:(CBCentralManager *)central didFailToConnectPeripheral:(CBPeripheral *)aPeripheral error:(NSError *)error
{
    NSLog(@"Fail to connect to peripheral: %@ with error = %@", aPeripheral, [error localizedDescription]);
    [connectButton setTitle:@"Connect"];
    if( peripheral )
    {
        [peripheral setDelegate:nil];
        peripheral = nil;
    }
}

#pragma mark - CBPeripheral delegate methods
/*
 Invoked upon completion of a -[discoverServices:] request.
 Discover available characteristics on interested services
 */
- (void) peripheral:(CBPeripheral *)aPeripheral didDiscoverServices:(NSError *)error
{
    for (CBService *aService in aPeripheral.services)
    {
        NSLog(@"Service found with UUID: %@", aService.UUID);
        
        /* Heart Rate Service */
        if ([aService.UUID isEqual:[CBUUID UUIDWithString:@"03B80E5A-EDE8-4B33-A751-6CE34EC4C700"]])
        {
            [aPeripheral discoverCharacteristics:nil forService:aService];
        }
        
        /* Device Information Service */
        if ([aService.UUID isEqual:[CBUUID UUIDWithString:@"180A"]])
        {
            [aPeripheral discoverCharacteristics:nil forService:aService];
        }
        
        /* GAP (Generic Access Profile) for Device Name */
        if ( [aService.UUID isEqual:[CBUUID UUIDWithString:CBUUIDGenericAccessProfileString]] )
        {
            [aPeripheral discoverCharacteristics:nil forService:aService];
        }
    }
}

/*
 Invoked upon completion of a -[discoverCharacteristics:forService:] request.
 Perform appropriate operations on interested characteristics
 */
- (void) peripheral:(CBPeripheral *)aPeripheral didDiscoverCharacteristicsForService:(CBService *)service error:(NSError *)error
{
    NSLog(@"Found Service: %@", service.UUID);
    if ([service.UUID isEqual:[CBUUID UUIDWithString:@"03B80E5A-EDE8-4B33-A751-6CE34EC4C700"]])
    {
        for (CBCharacteristic *aChar in service.characteristics)
        {
            
            /* Read body sensor location */
            if ([aChar.UUID isEqual:[CBUUID UUIDWithString:@"03B80E5A-EDE8-4B33-A751-6CE34EC4C700"]])
            {
                
                [aPeripheral readValueForCharacteristic:aChar];
                NSLog(@"Found MIDI Device");
            }
            [aPeripheral setNotifyValue:YES forCharacteristic:aChar];
            /* Write heart rate control point */
            if ([aChar.UUID isEqual:[CBUUID UUIDWithString:@"2A39"]])
            {
                uint8_t val = 1;
                NSData* valData = [NSData dataWithBytes:(void*)&val length:sizeof(val)];
                [aPeripheral writeValue:valData forCharacteristic:aChar type:CBCharacteristicWriteWithResponse];
            }
        }
    }
    
    if ( [service.UUID isEqual:[CBUUID UUIDWithString:CBUUIDGenericAccessProfileString]] )
    {
        for (CBCharacteristic *aChar in service.characteristics)
        {
            /* Read device name */
            if ([aChar.UUID isEqual:[CBUUID UUIDWithString:CBUUIDDeviceNameString]])
            {
                [aPeripheral readValueForCharacteristic:aChar];
                NSLog(@"Found a Device Name Characteristic");
            }
        }
    }
    
    if ([service.UUID isEqual:[CBUUID UUIDWithString:@"180A"]])
    {
        for (CBCharacteristic *aChar in service.characteristics)
        {
            /* Read manufacturer name */
            if ([aChar.UUID isEqual:[CBUUID UUIDWithString:@"2A29"]])
            {
                [aPeripheral readValueForCharacteristic:aChar];
                NSLog(@"Found a Device Manufacturer Name Characteristic");
            }
        }
    }
}

/*
 Invoked upon completion of a -[readValueForCharacteristic:] request or on the reception of a notification/indication.
 */
- (void) peripheral:(CBPeripheral *)aPeripheral didUpdateValueForCharacteristic:(CBCharacteristic *)characteristic error:(NSError *)error
{
  //  NSLog(@"MIDI Output: %@",characteristic.value);
    [self.debugView insertText:[NSString stringWithFormat:@"%@ \r", [characteristic.value hexRepresentationWithSpaces_AS:YES]]];
    NSRange range = NSMakeRange(2, [characteristic.value length] -2);
    NSMutableData *refinedData = [NSMutableData dataWithData:[characteristic.value subdataWithRange:range]];
    [self.debugView insertText:[NSString stringWithFormat:@"Length: %lu, %@ \r", (unsigned long)[refinedData length],[refinedData hexRepresentationWithSpaces_AS:YES]]];
    NSMutableData *newData = [[NSMutableData alloc]init];
    int x = 1;
    int pos = 3;
    if ([refinedData length] > 3) {
        [refinedData replaceBytesInRange:NSMakeRange(3, 1) withBytes:NULL length:0];
        Byte *byteData = (Byte*)malloc(3);
        
        [refinedData getBytes:&byteData range:NSMakeRange(x, 3)];
        pos = 3*x +2;
        [newData appendBytes:&byteData length:3];
      //  NSUInteger len = [refinedData length];
      //  Byte *byteData = (Byte*)malloc(len);
      //  [refinedData getBytes:&byteData length:len];
        
    }
    
    NSUInteger length = [refinedData length];
    NSUInteger chunkSize = 3;
    NSUInteger offset = 0;
    do {
        NSUInteger thisChunkSize = length - offset > chunkSize ? chunkSize : length - offset;
        NSData* chunk = [NSData dataWithBytesNoCopy:(char *)[refinedData bytes] + offset
                                             length:thisChunkSize
                                       freeWhenDone:NO];
        offset += thisChunkSize;
        [self.debugView insertText:[NSString stringWithFormat:@"Chunky: %lu, %@ \r", (unsigned long)[chunk length],[chunk hexRepresentationWithSpaces_AS:YES]]];
        NSUInteger len = [chunk length];
        const uint8 *bytes = (const uint8*)[chunk bytes];
        [midi sendBytes:bytes size:sizeof(bytes)];
        // do something with chunk
    } while (offset < length);
    [self.debugView insertText:[NSString stringWithFormat:@"Fixed Data: %lu, %@ \r", (unsigned long)[newData length],[refinedData hexRepresentationWithSpaces_AS:YES]]];
    
 //   [self MySendMidi:bytes size:sizeof(bytes)];
    // [self sendBytes:bytes size:sizeof(bytes)];
  //  [self sendBytes:bytes size:sizeof(bytes)];
   // pkt = MIDIPacketListInit(pktList);
  //  pkt = MIDIPacketListInit(pktList);
  //  pkt = MIDIPacketListAdd(pktList, 1024, pkt, 0, 3, midiDataToSend);
   
    
    
   // NSUInteger len = [refinedData length];
 //  Byte *byteData = (Byte*)malloc(len);
   // [refinedData getBytes:&byteData length:len];
  // memcpy(byteData, [refinedData bytes], len);
 //   free(byteData);
  //  [midi sendBytes:bytes size:sizeof(bytes)];
  //  MIDITimeStamp timestamp = 0;
  //  MIDIPacket     *currentpacket = MIDIPacketListInit(pktList);
 //   currentpacket = MIDIPacketListAdd(pktList, 1024, currentpacket, timestamp, sizeof(byteData)/sizeof(byteData[0]), byteData);
  
   // [self sendPacketList:pktList toDestination:[midi.destinations objectAtIndex:0]];
    
    /* Updated value for heart rate measurement received */
    if ([characteristic.UUID isEqual:[CBUUID UUIDWithString:@"2A37"]])
    {
        if( (characteristic.value)  || !error )
        {
            /* Update UI with heart rate data */
            [self updateWithHRMData:characteristic.value];
        }
    }
    /* Value for body sensor location received */
    else  if ([characteristic.UUID isEqual:[CBUUID UUIDWithString:@"2A38"]])
    {
        NSData * updatedValue = characteristic.value;
        uint8_t* dataPointer = (uint8_t*)[updatedValue bytes];
        if(dataPointer)
        {
            uint8_t location = dataPointer[0];
            NSString*  locationString;
            }
    }
    /* Value for device Name received */
    else if ([characteristic.UUID isEqual:[CBUUID UUIDWithString:CBUUIDDeviceNameString]])
    {
        NSString * deviceName = [[NSString alloc] initWithData:characteristic.value encoding:NSUTF8StringEncoding];
        NSLog(@"Device Name = %@", deviceName);
    }
    /* Value for manufacturer name received */
    else if ([characteristic.UUID isEqual:[CBUUID UUIDWithString:@"2A29"]])
    {
        self.manufacturer = [[NSString alloc] initWithData:characteristic.value encoding:NSUTF8StringEncoding];
        NSLog(@"Manufacturer Name = %@", self.manufacturer);
    }
}

-(void)addString:(NSString *)text {
    NSLog(text);
}

- (void) midi:(PGMidi*)midi sourceAdded:(PGMidiSource *)source
{
    [source addDelegate:self];
    [self addString:[NSString stringWithFormat:@"Source added: %@", ToString(source)]];
}

- (void) midi:(PGMidi*)midi sourceRemoved:(PGMidiSource *)source
{
   // [self updateCountLabel];
    [self.sources removeObject:source];
    [self addString:[NSString stringWithFormat:@"Source removed: %@", ToString(source)]];
}

- (void) midi:(PGMidi*)midi destinationAdded:(PGMidiDestination *)destination
{

    [self addString:[NSString stringWithFormat:@"Desintation added: %@", ToString(destination)]];
}

- (void) midi:(PGMidi*)midi destinationRemoved:(PGMidiDestination *)destination
{


    [self addString:[NSString stringWithFormat:@"Desintation removed: %@", ToString(destination)]];
}

NSString *StringFromPacket(const MIDIPacket *packet)
{
    // Note - this is not an example of MIDI parsing. I'm just dumping
    // some bytes for diagnostics.
    // See comments in PGMidiSourceDelegate for an example of how to
    // interpret the MIDIPacket structure.
    return [NSString stringWithFormat:@"  %u bytes: [%02x,%02x,%02x]",
            packet->length,
            (packet->length > 0) ? packet->data[0] : 0,
            (packet->length > 1) ? packet->data[1] : 0,
            (packet->length > 2) ? packet->data[2] : 0
            ];
}

const char *CToString(BOOL b) { return b ? "yes":"no"; }

NSString *ToString(PGMidiConnection *connection)
{
    return [NSString stringWithFormat:@"< PGMidiConnection: name=%@ isNetwork=%s >",
            connection.name, CToString(connection.isNetworkSession)];
}
- (void) setMidi:(PGMidi*)m
{
    midi.delegate = nil;
    midi = m;
    midi.delegate = self;
    
    [self attachToAllExistingSources];
}
- (void) attachToAllExistingSources
{
    for (PGMidiSource *source in midi.sources)
    {
        NSLog(@"Attaching to: %@", source.name);
        [source addDelegate:self];
    }
}

- (void) sendMidiDataInBackground
{
    for (int n = 0; n < 20; ++n)
    {
        /* const UInt8 note      = RandomNoteNumber();
         const UInt8 noteOn[]  = { 0x90, note, 127 };
         const UInt8 noteOff[] = { 0x80, note, 0   };
         
         [midi sendBytes:noteOn size:sizeof(noteOn)];
         [NSThread sleepForTimeInterval:0.1];
         [midi sendBytes:noteOff size:sizeof(noteOff)]; */
    }
}

-(void)sendPacketList:(MIDIPacketList *)packetList toDestination:(PGMidiDestination *)dest
{
    [dest sendPacketList:packetList];
}

@end
