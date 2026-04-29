# Reference materials

This branch carries third-party reference materials used during development of the WCH-LinkE CMSIS-DAP firmware on [`fw-hs-wch-wchlinkE`](../../tree/fw-hs-wch-wchlinkE). They are kept here for **learning and study purposes only** — no code path on that firmware branch depends on anything in this tree.

## Sources

These materials originate from different upstream vendors:

- **WCH (Nanjing Qinheng Microelectronics)** — EVT examples, peripheral reference code, datasheets, and reference manuals from the MounRiver SDK and the official WCH documentation pages.
- **Microsoft** — `usb/MS_OS_2_0_desc.docx`, the proprietary specification document for the OS 2.0 descriptor system used by WinUSB and driverless device binding.

Each file retains its original headers, metadata, and copyright notices where present.

## License

We respect the licenses and terms of every upstream source. Inclusion of these materials on this branch is solely for educational reference during development of this firmware project.

If you believe any file in this tree is included in a way that violates its license or the rights of its author, please **open an issue on this repository** so it can be removed or relocated promptly.

## How this branch fits

This `ref-hs-wch-wchlinkE` branch is a standalone tree of supporting materials and does not share history with any other branch. The repository's `main` branch is an index of all firmware ports (see its [README.md](../../tree/main)); the actual WCH-LinkE firmware lives on [`fw-hs-wch-wchlinkE`](../../tree/fw-hs-wch-wchlinkE).

To browse the reference materials:

```bash
git fetch origin ref-hs-wch-wchlinkE
git checkout ref-hs-wch-wchlinkE
```
