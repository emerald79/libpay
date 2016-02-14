# LibEMV - The Toolkit for Smart Payment Applications

## LibEMV Configuration

### LibEMV Configuration -- Data Dictionary

Name                 | Description                                           | Source   | Format | Template   | Tag        | Length 
-------------------- | ----------------------------------------------------- | -------- | ------ | ---------- | ---------- | ------
Configuration        | Configuration of Entry Point and Level 2 Kernels      | Terminal | var.   | --         | 'FF81E371' | var.
Set of Combinations  | Set of Combinations of AID and Kernel ID              | Terminal | var.   | 'FF81E371' | 'FF82E371' | var.
Transaction Types    | Transaction Types the Set of Combinations apply to    | Terminal | b      | 'FF82E371' | 'DF84E371' | 1 - 4

### LibEMV Configuration -- Structure

Tag        | Name          | --                  | --                | Mandatory or Optional
---------- | ------------- | ------------------- | ----------------- | ---------------------
'FF81E371' | Configuration | --                  | --                | M
--         | 'FF82E371'    | Set of Combinations | --                | M
--         | --            | 'DF83E371'          | Transaction Types | M
