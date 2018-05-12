# Developers developers developers

Here is the developers page

Environment variables that must be set

`BSD_INC` - FreeBSD based include headers, a compatible copy can be found at freebsd-headers

You can set this on windows using `SET BSD_INC="path"` or on linux using `export BSD_INC="path"`

`ONI_FRAMEWORK` - Absolute path, or relative path (to current building project) to the OniFramework directory

To get started, from the `Mira` root directory run `Scripts/init_development_environment.cmd` if you are on windows or `Scripts/init_development_environment.sh` if you are on linux.

This will parse the configuration file and set the environment variables accordingly.

`REMOTE_BUILD_DIR` ~/projects
`MIRA_DIR` same jawn