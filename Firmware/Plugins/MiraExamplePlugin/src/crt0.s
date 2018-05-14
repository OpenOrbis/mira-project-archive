.intel_syntax noprefix
.text

.quad plugin_size
.quad plugin_initialize

.global _start

# Plugins should always spinloop
_start:
	jmp		_start
