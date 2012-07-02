# Microsoft Developer Studio Generated NMAKE File, Format Version 4.20
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

!IF "$(CFG)" == ""
CFG=IRScope - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to IRScope - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "IRScope - Win32 Release" && "$(CFG)" !=\
 "IRScope - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "IRScope.mak" CFG="IRScope - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "IRScope - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "IRScope - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 
################################################################################
# Begin Project
# PROP Target_Last_Scanned "IRScope - Win32 Debug"
RSC=rc.exe
CPP=cl.exe
MTL=mktyplib.exe

!IF  "$(CFG)" == "IRScope - Win32 Release"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
OUTDIR=.\Release
INTDIR=.\Release

ALL : "$(OUTDIR)\IRScope.exe"

CLEAN : 
	-@erase "$(INTDIR)\audio.obj"
	-@erase "$(INTDIR)\IRScope.obj"
	-@erase "$(INTDIR)\IRScope.pch"
	-@erase "$(INTDIR)\IRScope.res"
	-@erase "$(INTDIR)\IRScopeConfigDlg.obj"
	-@erase "$(INTDIR)\IRScopeDlg.obj"
	-@erase "$(INTDIR)\IrScopeWnd.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\widget.obj"
	-@erase "$(OUTDIR)\IRScope.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /c
CPP_PROJ=/nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D\
 "_MBCS" /Fp"$(INTDIR)/IRScope.pch" /Yu"stdafx.h" /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/IRScope.res" /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/IRScope.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 winmm.lib /nologo /subsystem:windows /machine:I386
LINK32_FLAGS=winmm.lib /nologo /subsystem:windows /incremental:no\
 /pdb:"$(OUTDIR)/IRScope.pdb" /machine:I386 /out:"$(OUTDIR)/IRScope.exe" 
LINK32_OBJS= \
	"$(INTDIR)\audio.obj" \
	"$(INTDIR)\IRScope.obj" \
	"$(INTDIR)\IRScope.res" \
	"$(INTDIR)\IRScopeConfigDlg.obj" \
	"$(INTDIR)\IRScopeDlg.obj" \
	"$(INTDIR)\IrScopeWnd.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\widget.obj"

"$(OUTDIR)\IRScope.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "IRScope - Win32 Debug"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "$(OUTDIR)\IRScope.exe"

CLEAN : 
	-@erase "$(INTDIR)\audio.obj"
	-@erase "$(INTDIR)\IRScope.obj"
	-@erase "$(INTDIR)\IRScope.pch"
	-@erase "$(INTDIR)\IRScope.res"
	-@erase "$(INTDIR)\IRScopeConfigDlg.obj"
	-@erase "$(INTDIR)\IRScopeDlg.obj"
	-@erase "$(INTDIR)\IrScopeWnd.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(INTDIR)\widget.obj"
	-@erase "$(OUTDIR)\IRScope.exe"
	-@erase "$(OUTDIR)\IRScope.ilk"
	-@erase "$(OUTDIR)\IRScope.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /c
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /D "_MBCS" /Fp"$(INTDIR)/IRScope.pch" /Yu"stdafx.h" /Fo"$(INTDIR)/"\
 /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/IRScope.res" /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/IRScope.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 winmm.lib /nologo /subsystem:windows /debug /machine:I386
LINK32_FLAGS=winmm.lib /nologo /subsystem:windows /incremental:yes\
 /pdb:"$(OUTDIR)/IRScope.pdb" /debug /machine:I386 /out:"$(OUTDIR)/IRScope.exe" 
LINK32_OBJS= \
	"$(INTDIR)\audio.obj" \
	"$(INTDIR)\IRScope.obj" \
	"$(INTDIR)\IRScope.res" \
	"$(INTDIR)\IRScopeConfigDlg.obj" \
	"$(INTDIR)\IRScopeDlg.obj" \
	"$(INTDIR)\IrScopeWnd.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\widget.obj"

"$(OUTDIR)\IRScope.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

################################################################################
# Begin Target

# Name "IRScope - Win32 Release"
# Name "IRScope - Win32 Debug"

!IF  "$(CFG)" == "IRScope - Win32 Release"

!ELSEIF  "$(CFG)" == "IRScope - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\ReadMe.txt

!IF  "$(CFG)" == "IRScope - Win32 Release"

!ELSEIF  "$(CFG)" == "IRScope - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\IRScope.cpp
DEP_CPP_IRSCO=\
	".\IRScope.h"\
	".\IRScopeDlg.h"\
	".\IrScopeWnd.h"\
	".\StdAfx.h"\
	

"$(INTDIR)\IRScope.obj" : $(SOURCE) $(DEP_CPP_IRSCO) "$(INTDIR)"\
 "$(INTDIR)\IRScope.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\IRScopeDlg.cpp
DEP_CPP_IRSCOP=\
	".\audio.h"\
	".\IRScope.h"\
	".\IRScopeDlg.h"\
	".\IrScopeWnd.h"\
	".\StdAfx.h"\
	".\wavfile.h"\
	".\widget.h"\
	

"$(INTDIR)\IRScopeDlg.obj" : $(SOURCE) $(DEP_CPP_IRSCOP) "$(INTDIR)"\
 "$(INTDIR)\IRScope.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\StdAfx.cpp
DEP_CPP_STDAF=\
	".\StdAfx.h"\
	

!IF  "$(CFG)" == "IRScope - Win32 Release"

# ADD CPP /Yc"stdafx.h"

BuildCmds= \
	$(CPP) /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS"\
 /Fp"$(INTDIR)/IRScope.pch" /Yc"stdafx.h" /Fo"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\StdAfx.obj" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\IRScope.pch" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "IRScope - Win32 Debug"

# ADD CPP /Yc"stdafx.h"

BuildCmds= \
	$(CPP) /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /D "_MBCS" /Fp"$(INTDIR)/IRScope.pch" /Yc"stdafx.h" /Fo"$(INTDIR)/"\
 /Fd"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\StdAfx.obj" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\IRScope.pch" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\IRScope.rc
DEP_RSC_IRSCOPE=\
	".\res\idr_irsc.ico"\
	".\res\idr_main.ico"\
	".\res\IRScope.rc2"\
	".\res\license1.bin"\
	".\res\manifest.txt"\
	

"$(INTDIR)\IRScope.res" : $(SOURCE) $(DEP_RSC_IRSCOPE) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\IrScopeWnd.cpp
DEP_CPP_IRSCOPEW=\
	".\IRScope.h"\
	".\IrScopeConfigDlg.h"\
	".\IrScopeWnd.h"\
	".\StdAfx.h"\
	

"$(INTDIR)\IrScopeWnd.obj" : $(SOURCE) $(DEP_CPP_IRSCOPEW) "$(INTDIR)"\
 "$(INTDIR)\IRScope.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\IRScopeConfigDlg.cpp
DEP_CPP_IRSCOPEC=\
	".\IRScope.h"\
	".\IrScopeConfigDlg.h"\
	".\StdAfx.h"\
	

"$(INTDIR)\IRScopeConfigDlg.obj" : $(SOURCE) $(DEP_CPP_IRSCOPEC) "$(INTDIR)"\
 "$(INTDIR)\IRScope.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\widget.cpp
DEP_CPP_WIDGE=\
	".\IrScopeWnd.h"\
	".\StdAfx.h"\
	".\widget.h"\
	

"$(INTDIR)\widget.obj" : $(SOURCE) $(DEP_CPP_WIDGE) "$(INTDIR)"\
 "$(INTDIR)\IRScope.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\audio.cpp
DEP_CPP_AUDIO=\
	".\audio.h"\
	".\wavfile.h"\
	

"$(INTDIR)\audio.obj" : $(SOURCE) $(DEP_CPP_AUDIO) "$(INTDIR)"\
 "$(INTDIR)\IRScope.pch"


# End Source File
# End Target
# End Project
################################################################################
