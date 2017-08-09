#ifndef CONFIG_H
#define CONFIG_H

/* Define INDI Data Dir */
#cmakedefine INDI_DATA_DIR "@INDI_DATA_DIR@"

/* Define Driver version */
#define GENERIC_VERSION_MAJOR @GENERIC_VERSION_MAJOR@
#define GENERIC_VERSION_MINOR @GENERIC_VERSION_MINOR@

/* Endianness check */
#define IS_BIG_ENDIAN @IS_BIG_ENDIAN@

#endif // CONFIG_H
