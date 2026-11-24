# W4 Office Desktop

W4 Office is an office suite for working with local documents, spreadsheets,
presentations, PDF files, forms, and diagrams. It includes editors for DOCX,
XLSX, and PPTX, and can be used offline.

## Download

Get W4 Office for Windows x64 from [GitHub Releases](https://github.com/W4MEX/W4-OfficeDesktop/releases).

- **Installer:** run the Windows setup file.
- **Portable USB edition:** extract the complete `W4 Office USB` folder and run
  `W4OfficeUSB.exe` from inside it.
- Verify the downloads against the `SHA256SUMS.txt` attached to the release.

## Source

- [W4 DesktopApps](https://github.com/W4MEX/W4-DesktopApps) provides the native
  Windows interface and installer resources.
- [W4 Office WebApps](https://github.com/W4MEX/W4-Office-WebApps) provides the
  editors and the `w4-office` theme.
- [W4 Office Server](https://github.com/W4MEX/W4-OfficeServer) is the companion
  server project.

This repository pins its build dependencies as Git submodules. See the
[attribution notices](ATTRIBUTION) for the upstream projects.

## Build

```sh
git clone https://github.com/W4MEX/W4-OfficeDesktop.git
cd W4-OfficeDesktop
git submodule update --init --recursive
```

See the [build overview](build/README.md), [Windows build instructions](build/windows/README.md),
and [Windows and USB r3 notes](build/windows/usb-r3.md). Windows x64 is the
published W4 release target.

CI uses the W4 Office profile in [branding.hcl](build/w4/branding.hcl). The
native assets and editor theme stay in the W4 forks. When updating submodule
commits, review the update and check the resulting W4 artifacts before release.

## Contribute

Report a bug or suggest a feature in [W4 Office Desktop Issues](https://github.com/W4MEX/W4-OfficeDesktop/issues).
For source changes, open a pull request in the repository that owns the affected
component.

## License

See [LICENSE](LICENSE) and [ATTRIBUTION](ATTRIBUTION) for the license and credits.
