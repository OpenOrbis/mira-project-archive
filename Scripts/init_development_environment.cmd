@echo OFF

REM We will start by parsing the configuration

REM The path of the configuration file, it should be within this directory
@SET MIRA_CONFIG_PATH=Scripts/config.txt
@SET MIRA_ERR=ERR:
@SET MIRA_WARN=WARN:
@SET MIRA_INFO=INFO:
@SETLOCAL ENABLEDELAYEDEXPANSION

REM Checks to see if the configuration file exists
if EXIST %MIRA_CONFIG_PATH% (
	for /f "delims=" %%i in (%MIRA_CONFIG_PATH%) do SET "%%i"
) else (
	REM Let the user know why the configuration script errored
	@echo "%MIRA_ERR% Configuration file not found (%MIRA_CONFIG_PATH%)."

	REM Return with an error, file not found
	goto :eof
)

REM Output all of the settings that we have imported from the configuration file
@echo "Environment variables configured:"
@echo "Mira Remote Directory: (%MIRA_REMOTE_DIR%)"
@echo "Mira Remote Output Directory: (%MIRA_REMOTE_OUTPUT_DIR%)"

REM Verify that the MIRA_DIR directory actually exists
if NOT EXIST "%MIRA_DIR%" (
	@echo "%MIRA_ERR% 'MIRA_DIR' environment variable not set in Scripts/config.txt"
	@echo "Directory not found"

	goto :eof
) else (
	@echo "Mira Directory: (%MIRA_DIR%)"
)

if NOT EXIST "%ONI_FRAMEWORK%" (
	@echo "%MIRA_ERR% 'ONI_FRAMEWORK' environment variable not set in the Scripts/config.txt"
	@echo "Directory not found"

	goto :eof
) else (
	@echo "OniFramework directory: %ONI_FRAMEWORK%"
)

REM Handle the launching of visual studio with our environment variables pre-set
if "%1"=="-launchvs" (
	call "%VS140COMNTOOLS%/vsvars32.bat" 
	dev "%MIRA_DIR%/Mira.sln"
)