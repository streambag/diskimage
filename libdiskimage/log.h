#ifndef LDI_LOG_H
#define LDI_LOG_H

/*
 * Ideally, the format argument should not be part of VA_ARGS, but if
 * we define it explicitly things will fail when there are zero extra arguments.
 */
#define LOG(logger, level, ...)	logger.write(level, logger.privarg,  __VA_ARGS__)

#define LOG_ERROR(logger, ...)	LOG(logger, 1,  __VA_ARGS__)
#define LOG_WARNING(logger, ...)	LOG(logger, 2, __VA_ARGS__)
#define LOG_INFO(logger, ...)	LOG(logger, 3, __VA_ARGS__)
#define LOG_VERBOSE(logger, ...)	LOG(logger, 4, __VA_ARGS__)

#endif					/* LDI_LOG_H */
