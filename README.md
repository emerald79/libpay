# LibEMV - The Toolkit for Smart Payment Applications

## LibEMV Configuration

### LibEMV Configuration -- Data Dictionary

Name                 | Description                                           | Source   | Format | Template   | Tag        | Length 
-------------------- | ----------------------------------------------------- | -------- | ------ | ---------- | ---------- | ------
Configuration        | Configuration of Entry Point and Level 2 Kernels      | Terminal | var.   | --         | 'FF81E371' | var.
Set of Combinations  | Set of Combinations of AID and Kernel ID              | Terminal | var.   | 'FF81E371' | 'FF82E371' | var.
Transaction Types    | Transaction Types the Set of Combinations apply to    | Terminal | b      | 'FF82E371' | 'DF84E371' | 1 - 4

### LibEMV Configuration -- Structure

Tag        | Name          | --                  | --                                   | --        | Mandatory or Optional
---------- | ------------- | ------------------- | ------------------------------------ | --------- | ---------------------
'FF81E371' | Configuration | --                  | --                                   | --        | M
--         | 'FF82E371'    | Set of Combinations | --                                   | --        | M
--         | --            | 'DF83E371'          | Transaction Types                    | --        | M
--         | --            | 'FF84E371'          | Combination                          | --        | M
--         | --            | --                  | 'DF85E371'                           | AID       | M
--         | --            | --                  | 'DF86E371'                           | Kernel ID | M
--         | --            | 'FF84E371'          | Combination                          | --        | O
--         | --            | --                  | 'DF85E371'                           | AID       | M
--         | --            | --                  | 'DF86E371'                           | Kernel ID | M
--         | --            | 'DF87E371'          | Status Check Supported               | --        | O
--         | --            | 'DF88E371'          | Zero Amount Allowed                  | --        | O
--         | --            | 'DF89E371'          | Extended Selection Supported         | --        | O
--         | --            | 'DF8AE371'          | Reader Contactless Transaction Limit | --        | O
--         | --            | 'DF8BE371'          | Reader Contactless Floor Limit       | --        | O
--         | --            | 'DF8CE371'          | Terminal Floor Limit                 | --        | O
--         | --            | 'DF8DE371'          | Reader CVM Required Limit            | --        | O
--         | --            | 'DF8EE371'          | Terminal Transaction Qualifiers      | --        | O
--         | 'FF82E371'    | Set of Combinations | --                                   | --        | O
--         | --            | 'DF83E371'          | Transaction Types                    | --        | M
--         | --            | 'FF84E371'          | Combination                          | --        | M
--         | --            | --                  | 'DF85E371'                           | AID       | M
--         | --            | --                  | 'DF86E371'                           | Kernel ID | M
--         | --            | 'DF87E371'          | Status Check Supported               | --        | O
--         | --            | 'DF88E371'          | Zero Amount Allowed                  | --        | O
--         | --            | 'DF89E371'          | Extended Selection Supported         | --        | O
--         | --            | 'DF8AE371'          | Reader Contactless Transaction Limit | --        | O
--         | --            | 'DF8BE371'          | Reader Contactless Floor Limit       | --        | O
--         | --            | 'DF8CE371'          | Terminal Floor Limit                 | --        | O
--         | --            | 'DF8DE371'          | Reader CVM Required Limit            | --        | O
--         | --            | 'DF8EE371'          | Terminal Transaction Qualifiers      | --        | O
--         | 'FF8FE371'    | Autorun             | --                                   | --        | O
--         | --            | 'DF90E371'          | Transaction Type                     | --        | M
--         | --            | 'DF91E371'          | Amount Authorized                    | --        | M
--         | 'FF92E371'    | Terminal Data       | --                                   | --        | M
--         | --            | '9F01'              | Acquirer Identifier                  | --        | M
--         | --            | '9F15'              | Merchant Category Code               | --        | M
--         | --            | '9F16'              | Merchant Identifier                  | --        | M
--         | --            | '9F1A'              | Terminal Country Code                | --        | M
--         | --            | '9F1C'              | Terminal Identification              | --        | M
--         | --            | '9F35'              | Terminal Type                        | --        | M
--         | --            | '9F39'              | Point-Of-Service (POS) Entry Mode    | --        | M
--         | --            | '9F40'              | Additional Terminal Capabilities     | --        | M
--         | --            | '9F4E'              | Merchant Name and Location           | --        | M
