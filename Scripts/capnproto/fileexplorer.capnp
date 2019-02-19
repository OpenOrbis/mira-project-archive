# Mira File Explorer Header
# Created: Jan 27, 2019
#
@0xd35134bfe0754755;

#enum FileTransferCommands {
#    echo @0;
#    open @1;
#    close @2;
#    getDents @3;
#    read @4;
#    write @5;
#    unlink @6;
#    move @7;
#    stat @8;
#    mkDir @9;
#    rmDir @10;
#}

struct DirEnt {
    enum DirEntType {
        unknown @0;
        fifo @1;
        chr @2;
        invalid0 @3;
        dir @4;
        invalid1 @5;
        blk @6;
        invalid2 @7;
        reg @8;
        invalid3 @9;
        lnk @10;
        invalid4 @11;
        sock @12;
        invalid5 @13;
        wht @14;
    }

    fileno @0 :UInt32;
    reclen @1 :UInt32;
    type @2 :DirEntType;
    name @3 :Text;
}

struct EchoRequest {
    message @0 :Text;
}

struct EchoResponse {
    error @0 :Int32;
}

# open
struct OpenRequest {
    path @0 :Text;
    flags @1 :Int32;
    mode @2 :Int32;
}

struct OpenResponse {
    error @0 :Int32;
    handle @1 :Int32;
}

# close
struct CloseRequest {
    handle @0 :Int32;
}

struct CloseResponse {
    error @0 :Int32;
}

# read
struct ReadRequest {
    handle @0 :Int32;
    offset @1 :UInt32;
    size @2:UInt32;
}

struct ReadResponse {
    error @0 :Int32;
    bytes @1 :Data;
}

# write
struct WriteRequest {
   handle @0 :Int32;
   offset @1 :UInt32;
   bytes @2 :Data; 
}

# mkdir
struct MkDirRequest {
    path @0 :Text;
    mode @1 :Int32;
}

struct MkDirResponse {
    error @0 :Int32;
}

# rmdir
struct RmDirRequest {
    path @0 :Text;
    recursive @1 :UInt8;
}

struct RmDirResponse {
    error @0 :Int32;
}

# unlink
struct UnlinkRequest {
    path @0 :Text;
}

struct UnlinkResponse {
    error @0 :Int32;
}

# stat(s)
struct TimeSpec {
    sec @0 :Int64;
    nsec @1 :UInt64;
}

struct StatRequest {
    handle @0 :Int32 = -1;
    path @1 :Text;
}

struct StatResponse {
    error @0 :Int32;
    path @1 :Text;
    dev @2 :UInt32;
    ino @3 :UInt32;
    mode @4 :UInt32;
    nlink @5 :UInt32;
    uid @6 :UInt32;
    gid @7 :UInt32;
    rdev @8 :UInt32;
    atim @9 :TimeSpec;
    mtim @10 :TimeSpec;
    ctim @11 :TimeSpec;
    size @12 :Int64;
    blocks @13 :Int64;
    blksize @14 :UInt32;
    flags @15 :UInt32;
    gen @16 :UInt32;
    lspare @17 :Int32;
    birthtim @18 :TimeSpec;
}

# getdents
struct GetDentsRequest {
    path @0 :Text;
}

struct GetDentsResponse {
    error @0 :Int32;
    entries @1 : List(DirEnt);
}