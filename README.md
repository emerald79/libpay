LibEMV - The Toolkit for Smart Payment Applications
===================================================

Name                 | Description                                      | Source   | Format | Template   | Tag        | Length
-------------------- | ------------------------------------------------ | -------- | ------ | ---------- | ---------- | ------
Configuration        | Configuration of Entry Point and Level 2 Kernels | Terminal | var.   | --         | 'FF81E371' | var.
Set of Combinations  | Set of Combinations of AID and Kernel            | Terminal | var.   | 'FF81E371' | 'FF82E371' | var.

---------- | ------------- | ------------------- | ----------------- | -
'FF81E371' | Configuration | -                   | -                 | M
-          | 'FF82E371'    | Set of Combinations | -                 | M
-          | -             | 'DF83E371'          | Transaction Types | M
