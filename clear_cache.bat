@echo off

@RD /S /Q "Intermediate"
@RD /S /Q "Binaries"
@RD /S /Q "Saved"
@RD /S /Q "DerivedDataCache"
@RD /S /Q ".vs"

@DEL "Project.sln"
