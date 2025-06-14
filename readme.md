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

