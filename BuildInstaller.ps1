"const char c_gitVersion[] = ""$(git describe --tags)"";" | Set-Content -Path ./TheForceEngine/gitVersion.h
cmake.exe -G"Visual Studio 17 2022" -A x64 -B ./BuildArtifacts/Build -DENABLE_VR=ON -DSTART_VR=ON
cmake --build ./BuildArtifacts/Build --config RelWithDebInfo
cmake --install ./BuildArtifacts/Build --config RelWithDebInfo --prefix ./BuildArtifacts/TheForceEngine
curl -o vc_redist.x64.exe https://aka.ms/vs/17/release/vc_redist.x64.exe
& "c:\Program Files (x86)\Inno Setup 6\ISCC.exe" /Qp /DAppVersionNumber="$((git describe --tags).Trim())" /DMSVCPath="$pwd" /DProgramPath="$pwd/BuildArtifacts/TheForceEngine" "$pwd/TheForceEngine/Installer/Installer.iss"
del vc_redist.x64.exe
# pause