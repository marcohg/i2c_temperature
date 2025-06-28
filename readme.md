# I2C Temperature Using iMXRT1010

## git 
- add board/ CMSIS/ component/ device/ drivers/ source/ startup/ utilities/ xip/ .settings/
- add .cproject  .gitignore .project readme.md 

## First commit
- create a new c/c++ project and add drivers for gpt and i2c
- import the example files i2c.* and make it work

## Introduce Pin Tools
- Use pin tools to IOMUXC_SetPinConfig
- Note: **Enable Software Input ON** SION, abstracto from AI 
> In essence, setting the SION bit ensures that the I2C peripheral can correctly 
handle the bidirectional nature of the SDA line by forcing the input path to be active, 
regardless of the pin's default direction based on its alternate function setting

## Introduce peripheral i2c, master mode, 100KHz, transfer, clock wip
- Enable Open Drain, 22K pull up
- config word for pads: 0xD8A0U (just to check)

## Working I2C Clock
1. Windows->Prefernces->MCUXpresso config: Disable the "read only files", this protection messes up.
2. LPI2C clock tool: 24MHz ext ->PLL-> 480MHz -> /8 = 60MHz -> /6 = 10MHz
3. I2C peripheral clock source: BOOT_CLOCK RUN 10Mz
4. With 1,2 and 3, frequency can be set. The displayed calculated value should be closer or equal.
-> next remove i2c and use Config tools only

## Make AHT21 Work
- Measure Temperature as INdicated in AHT21 
	- aht21_t struct
	- aht21_measurement(aht21_t *d) blocking function
	- propduction type - make a non-blocking state machine 
- print float support and c/c++
	- Linker Script: 
		- NewLibNano-Semihost,
		- Enable prinf-float,
	- c preprocessor:
		- PRINTF_FLOAT_ENABLE=1

## State Machine for AHT21
- Non-blocking function to measure every MEASUREMENT_PERIOD (1000)
- Added user-led pin GPIO_11 to monitor timming
    