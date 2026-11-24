# W4 Office 9.3.9.1 Windows and USB revision 3

## Publish a W4 release

Create the GitHub Release manually and attach the verified Windows installer,
USB archive, and SHA-256 checksums. Create the release tag from a commit that
already contains the manual release workflow changes. A tag pointing to an
older commit can still run the workflow definitions stored in that commit.

WinGet distribution is disabled until W4 has its own verified package
identifier and publishing workflow.

The core patch bounds the reads when encoding transparent pixels in font
thumbnails and reserves enough space for the encoded output.

From the repository root, initialize the pinned submodules:

```powershell
git submodule update --init --recursive
```

Build the Windows payload using the instructions in [README.md](README.md).
The Windows build script applies the patch to the core worktree, which makes
the core package fetcher rebuild from source. Take `graphics.dll` from the rebuilt
Windows x64 converter payload and use it to repackage the USB r2 ZIP:

```powershell
.\build\windows\repack-usb.ps1 -BaseZip .\W4-Office-9.3.9.1-USB-r2-x64.zip `
    -GraphicsDll .\converter\graphics.dll -OutputZip .\W4-Office-9.3.9.1-USB-r3-x64.zip
```

The script checks the resulting archive and prints its SHA-256. Use 7-Zip on
PATH or set `-SevenZip` to its executable path. To run the portable edition,
extract the complete `W4 Office USB` folder and launch `W4OfficeUSB.exe` from it.

Validation recorded on November 24, 2026:

- Windows r3 passed installation, DOCX open, edit, save, reopen, conversion to
  PDF/TXT through the W4 converter, and uninstallation in Windows Sandbox.
  The saved document was preserved. Export through the graphical menu was
  outside that test.
- USB r3 opened and rendered the DOCX from an extracted guest folder on clean
  Windows without a preinstalled Visual C++ redistributable. Launch through
  a simulated drive reported that Windows could not load `chrome_elf.dll`.
  The DLL was present in the ZIP and the extracted folder.
- The maintainer confirmed normal operation from a physical USB. The
  `graphics.dll` on that copy matched the corrected build. Editing, saving
  and preference relocation were not individually recorded for the USB test.
