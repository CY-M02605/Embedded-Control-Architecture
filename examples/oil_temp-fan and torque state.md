# Oil Temperature-Based Cooling and Torque Limiting Example

| Oil temperature | Cooling fan request | Torque limit |                      State_description                        |
|----------------:|--------------------:|-------------:|---------------------------------------------------------------|
| ? 80 ‹C         | 20%                 | 100%         | Normal operation with low cooling demand                      |
| 90 ‹C           | 40%                 | 100%         | Cooling demand begins to increase                             |
| 100 ‹C          | 65%                 | 100%         | High cooling demand                                           |
| 105 ‹C          | 80%                 | 95%          | Mind torque reduction                                         |
| 110 ‹C          | 100%                | 80%          | Active overheat protection                                    |
| 115 ‹C          | 100%                | 60%          | Severe overheat condition                                     |
| ? 120 ‹C        | 100%                | 30%          | Critical overheat condition; machine shutdown may be required |

> This table is a simplified learning example and does not represent calibration data from a specific production vehicle.
