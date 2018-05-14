.intel_syntax noprefix
.text

.global _pluginInitialize
.global _pluginSize
.global _plugin

_pluginInitialize:
.quad plugin_initialize

_pluginSize:
.quad 0

.global _start

# Plugins should always spinloop
_start:
	jmp		_start