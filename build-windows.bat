call "%VS100COMNTOOLS%vsvars32.bat"

set DFHACKDIR="C:\Users\tomjo\Documents\code\dfhack-4203-alpha1"
set DFHACKVER=0.42.03-alpha1
set DFVERNUM=04203
set TWBT_VER=5.52-alpha

msbuild /p:Platform=Win32 /p:Configuration=Release /p:dfhack=%DFHACKDIR% /p:dfhackver=%DFHACKVER% /p:twbt_ver=%TWBT_VER% /p:dfvernum=%DFVERNUM% twbt.vcxproj

cd plugins
msbuild /p:Platform=Win32 /p:Configuration=Release /p:dfhack=%DFHACKDIR% /p:dfhackver=%DFHACKVER% /p:twbt_ver=%TWBT_VER% /p:dfvernum=%DFVERNUM% plugins.vcxproj

cd ..
