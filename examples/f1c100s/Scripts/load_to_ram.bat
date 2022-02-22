@cd %~dp0
@set jlink="jlink.exe"
@%jlink% -CommanderScript load_to_ram.jlink
