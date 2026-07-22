# ESP32 Flight Controller fabrication notes

- Build: 4-layer HDI 1+N+1, 1.60 mm finished thickness. The short USB interface
  is Full-Speed only; no controlled-impedance High-Speed claim is made.
- Stackup: 35 µm copper on all four layers; 0.20 mm FR-4 prepreg from L1-L2
  and L3-L4; 1.04 mm FR-4 core from L2-L3; nominal Dk 4.2.
- Minimum finished track/space: 0.15 mm / 0.15 mm.
- Through vias: 0.20 mm finished drill, 0.50 mm finished pad unless the drill file specifies otherwise.
- Laser microvias: 0.10 mm finished drill, 0.30 mm pad, limited to the outer
  buildup pairs L1-L2 and L4-L3. There are no L2-L3 microvias. The release has
  31 unique microvia sites and 133 unique drill sites in total.
- There are no via-in-pad features. Do not move any drill into an SMD land during CAM processing.
- U2 (MPU9250) has a via-free component courtyard. Preserve this area exactly; no via substitution or added stitching is allowed under the IMU package.
- Plug/tent all vias from both sides where the fabricator process permits. No
  stacked microvia or via-in-pad construction is present.
- ENIG surface finish; green LPI solder mask; white silkscreen.
- Electrical test: 100% flying-probe/netlist test against the supplied Gerbers.
- Inspect U2 land coplanarity and solder-mask registration before assembly. Reject any CAM-added drill, exposed hole, or mask intrusion in U2/C12 lands.
- The four large NPTH holes are the required 30.5 mm square ESC/frame mounting pattern; do not plate them.

Do not quote or fabricate this release on a process that cannot produce the specified 0.10 mm laser microvias and 0.15 mm track/space.
