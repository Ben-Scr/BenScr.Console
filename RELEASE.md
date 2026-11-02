# Release verification

Version: 1.0.0. NuGet ID: BenScr.Console. Release tag: packages/BenScr.Console/1.0.0.

Run tools/Verify.ps1 from a clean committed checkout. It checks source/header behavior, a relocated
CMake install, exact native package metadata/payloads and Windows x64 packages.config/vcxproj consumers
in Release and Debug. The .NET SDK project under package/ is packaging tooling only, not an assembly.

`artifacts/latest-verification.txt` identifies the successful run directory and release-manifest.json.
Only that run's explicitly named BenScr.Console.1.0.0.nupkg may be published. Confirm its SHA-256 and
the source commit/tag before upload. Preserve released tags and existing NuGet versions.
Do not publish a package from a dirty working tree or replace tested bytes without re-verification.

After publication, test the real public package with a new package directory:

```powershell
pwsh -File tools/VerifyNuGetConsumer.ps1 -Published -ExpectedPackage '<verified package path>'
```

This restores solely from nuget.org and compares the package's source/header/build/docs/license/icon
and nuspec payloads, allowing NuGet's additional signature envelope. Inspect the rendered public icon
and record the public URL, CI run and successful consumers before setting published_verified.

Package page: https://www.nuget.org/packages/BenScr.Console/1.0.0
Source/CI: https://github.com/Ben-Scr/BenScr.Console

Interactive terminal rendering and key-event encodings require host-specific checks and are not
claimed to be covered by the redirected automated tests. No AOT/interoperability or .NET support is claimed.
