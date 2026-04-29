# Notes for AI agents

This branch (`ref-hs-wch-wchlinkE`) is a **read-only reference tree** of vendor materials (WCH MCU / SDK extracts, Microsoft USB OS-2.0 descriptor spec) used during development of the firmware on [`fw-hs-wch-wchlinkE`](../../tree/fw-hs-wch-wchlinkE). See [README.md](README.md) for sources and licensing.

## What goes here

- Vendor MCU/peripheral documentation, EVT example code, datasheets — included verbatim from upstream with original headers, metadata, and copyright notices intact.
- Third-party USB / protocol specifications referenced by the firmware (e.g. Microsoft OS 2.0 descriptors).

## Editing rules

- **Do not modify vendor file content.** Each file retains its upstream form so it stays usable as a reference and so its provenance is verifiable. Reformatting, comment cleanup, line-ending normalization, etc. on vendor files are out of scope.
- **Do not add code that the firmware depends on.** This branch must remain non-load-bearing — the firmware on `fw-hs-wch-wchlinkE` builds without ever fetching this branch.
- **Editable on this branch:** [README.md](README.md), this file, and any future top-level index/notes that describe the materials. Changes to vendor subtrees (`mcu/`, `usb/`, etc.) should only be additive (importing new vendor material) or removal (if a license issue surfaces).
