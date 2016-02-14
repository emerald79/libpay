# LibEMV - The Toolkit for Smart Payment Applications

[TOC]

## LibEMV Configuration

### Data Dictionary for a LibEMV Configuration File

Name                 | Description                                           | Source   | Format | Template   | Tag        | Length
-------------------- | ----------------------------------------------------- | -------- | ------ | ---------- | ---------- | ------
Configuration        | Configuration of Entry Point and Level 2 Kernels      | Terminal | var.   | --         | 'FF81E371' | var.
Set of Combinations  | Set of Combinations of AID and Kernel                 | Terminal | var.   | 'FF81E371' | 'FF82E371' | var.
Transaction Types    | Transaction Types applying to the Set of Combinations | Terminal | b      | 'FF82E371' | 'DF84E371' | 1 - 4

### Structure of a LibEMV Configuration File

Tag        | Name          | --                  | --                | Mandatory or Optional
---------- | ------------- | ------------------- | ----------------- | ---------------------
'FF81E371' | Configuration | --                  | --                | M
--         | 'FF82E371'    | Set of Combinations | --                | M
--         | --            | 'DF83E371'          | Transaction Types | M
