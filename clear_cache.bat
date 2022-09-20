@echo off

@RD /S /Q "Intermediate"
@RD /S /Q "Binaries"
@RD /S /Q "Saved"
@RD /S /Q "Build"
@RD /S /Q "Releases"
@RD /S /Q "DerivedDataCache"
@RD /S /Q ".vs"

@DEL "Graphs.sln"
