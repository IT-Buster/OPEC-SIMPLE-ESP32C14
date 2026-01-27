# Summary of Changes - Advanced Modbus Configuration

## Overview
This PR adds comprehensive diagnostics and advanced configuration for Modbus communication to troubleshoot HT73 sensor connectivity issues.

## Files Modified

### 1. config.h
- **Added 4 new fields to Config struct:**
  - `uint16_t modbusStartRegister` - Starting register address (0-10)
  - `uint16_t modbusRegisterCount` - Number of registers to read (1-10)
  - `uint8_t modbusDataMapping` - Data interpretation mode (0-3)
  - `uint8_t modbusDivider` - Value divider (0=none, 1=÷10, 2=÷100)

### 2. OPEC-SIMPLE-ESP32C14.ino
- **Updated loadConfig():**
  - Load new fields from Preferences
  - Add validation for new fields
  
- **Updated saveConfig():**
  - Save new fields to Preferences
  
- **Updated resetConfig():**
  - Set default values for new fields
  - Default: FC03, start=0, count=2, mapping=HT73, divider=÷10

### 3. modbus_handler.h
- **Added extern declaration for ModbusMaster modbus**
- **Increased modbusRawRegisters array from 6 to 10 elements**
- **Added new function declarations:**
  - `void processModbusData()` - Process data according to mapping
  - `String scanModbusSlaves()` - Scan for Modbus devices

### 4. modbus_handler.cpp
- **Updated readModbusData():**
  - Use config parameters for register type, start, and count
  - Call processModbusData() after successful read
  
- **Implemented processModbusData():**
  - Apply divider based on config
  - Map data according to modbusDataMapping
  - Support for 4 mapping modes: HT73, HT73v2, PLC, Custom
  
- **Implemented scanModbusSlaves():**
  - Scan Slave IDs 1-10
  - Test both FC03 and FC04
  - Return JSON with results

### 5. web_server.cpp

#### HTML Changes:

**Settings Page:**
- Added new section "Konfiguracja Modbus - Zaawansowane"
- 5 new input fields for advanced configuration
- Updated JavaScript to load/save new fields

**Diagnostics Page:**
- Added "Narzędzia testowe Modbus" section with 3 buttons
- Added "Informacje diagnostyczne" section showing current config
- Updated table header (removed Binary, added Float/100)
- Changed table to show 10 registers instead of dynamic count

#### JavaScript Changes:

**Settings:**
- Updated `loadSettings()` to load 5 new fields
- Updated `saveSettings()` to save 5 new fields

**Diagnostics:**
- Updated `updateDisplay()` to handle 10 registers
- Added `scanSlaves()` - trigger scan and display results
- Added `testFC03()` - test Holding Registers
- Added `testFC04()` - test Input Registers
- Added `updateDiagnostics()` - load and display current config
- Added bounds checking for array access

#### API Endpoints:

- **GET /api/settings** - Extended to return 4 new fields
- **POST /api/settings** - Extended to save 4 new fields with validation
- **GET /api/modbus-scan** - New endpoint for scanning
- **GET /api/modbus-test?fc=3|4** - New endpoint for testing
- **GET /api/modbus-raw** - Updated to support 10 registers

### 6. ADVANCED_MODBUS_CONFIG.md (NEW)
- Comprehensive user guide in Polish
- Step-by-step instructions
- Example configurations for different devices
- Troubleshooting guide
- Technical documentation

## Key Features

### 1. Flexible Register Configuration
- Choose between FC03 (Holding) and FC04 (Input) registers
- Set custom start register (0-10)
- Set custom register count (1-10)

### 2. Data Mapping Options
- **HT73**: Reg0=Humidity, Reg1=Temperature
- **HT73v2**: Reg1=Temperature, Reg2=Humidity
- **PLC**: Reg0=Actuator, Reg4=TempEXT, Reg5=TempCO
- **Custom**: User-defined interpretation

### 3. Divider Options
- No division (raw values)
- Divide by 10 (most common for HT73)
- Divide by 100 (for some sensors)

### 4. Diagnostic Tools
- **Slave ID Scanner**: Automatically find devices (1-10)
- **Function Code Tests**: Test FC03 and FC04 separately
- **Live Config Display**: See current configuration
- **10-Register Table**: Extended register view

## Security & Quality

### Validations Added:
- Server-side validation of all new parameters
- Bounds checking in JavaScript array access
- Range validation (startReg ≤ 10, count 1-10, etc.)
- Default values for invalid configurations

### Documentation:
- Inline code comments explaining register offset behavior
- User-facing documentation (ADVANCED_MODBUS_CONFIG.md)
- Example configurations for common devices
- Troubleshooting guide

## Breaking Changes

**Default value change**: `modbusUseInputRegisters` default changed from `true` to `false`
- Old default: Input Registers (FC04)
- New default: Holding Registers (FC03)
- **Migration**: Existing users may need to change register type in settings if their device was working with FC04

## Testing Recommendations

1. **Basic Functionality:**
   - Load settings page - verify all fields appear
   - Save settings - verify values persist after restart
   - Load diagnostics page - verify all sections appear

2. **Scanning:**
   - Click "Skanuj Slave ID" - verify scan completes
   - Verify results show correct devices

3. **Testing:**
   - Click "Test FC03" - verify response
   - Click "Test FC04" - verify response

4. **Data Display:**
   - Verify Dashboard shows correct values
   - Verify Diagnostics table shows 10 registers
   - Verify register descriptions match mapping

5. **Edge Cases:**
   - Try invalid values in form fields
   - Try different mapping modes
   - Try different dividers
   - Verify validation prevents crashes

## API Changes Summary

### Modified Endpoints:
- `GET /api/settings` - Returns 4 additional fields
- `POST /api/settings` - Accepts 4 additional fields
- `GET /api/modbus-raw` - Now returns up to 10 registers

### New Endpoints:
- `GET /api/modbus-scan` - Returns JSON with scanned devices
- `GET /api/modbus-test?fc=3` - Test FC03 with current config
- `GET /api/modbus-test?fc=4` - Test FC04 with current config

## Backward Compatibility

- **Config structure**: Extended (not changed) - old configs will load with defaults
- **API**: Extended (not changed) - old clients will work
- **UI**: Extended (not changed) - new sections added, old ones unchanged
- **Default behavior**: Changed (FC04→FC03) - may require user action

## Performance Impact

- **Memory**: +16 bytes for new config fields
- **Register array**: +8 bytes (6→10 uint16_t)
- **Scan time**: ~1-2 seconds for scanning 10 slave IDs
- **Page size**: ~5KB increase in web_server.cpp

## Future Enhancements

Potential improvements not included in this PR:
1. Increase max startRegister beyond 10
2. Support for writing registers (testing)
3. Register map import/export
4. More mapping presets for other devices
5. Auto-detection of optimal settings
6. Historical data logging of scans

## References

- Original issue: Problem with HT73 sensor not responding via RS485
- Related: Missing UI for `modbusUseInputRegisters` field
- Modbus specs: FC03=Holding, FC04=Input registers
