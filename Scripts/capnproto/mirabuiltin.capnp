# Mira Builtin Header
# Created: Jan 27, 2019
#
@0xea90f3bd797e05a9;

enum MessageCategory {
    none @0;
    system @1;
    log @2;
    debug @3;
    file @4;
    cmd @5;
    max @6;
}
struct Message {
    category @0 :MessageCategory;
    type @1 :UInt32;
    containedMessage @2 :Text;
}