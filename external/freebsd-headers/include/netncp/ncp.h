/*
 *  ncp.h
 */
/*-
 *  Copyright (C) 1995 by Volker Lendecke
 *  New version derived from original ncp.h, 1998 Boris Popov
 *
 * $FreeBSD: release/9.0.0/sys/netncp/ncp.h 139823 2005-01-07 01:45:51Z imp $
 */

#ifndef _NETNCP_NCP_H_
#define _NETNCP_NCP_H_

#define NCP_VERMAJ	1
#define NCP_VERMIN	3500
#define NCP_VERSION	(NCP_VERMAJ*100000 + NCP_VERMIN)

typedef u_int32_t	nwdirent;

typedef char		nstr8;
typedef	nstr8*		pnstr8;
typedef u_int8_t	nuint8;
typedef u_int8_t*	pnuint8;
typedef u_int16_t	nuint16;
typedef	nuint16*	pnuint16;
typedef u_int32_t	nuint32;
typedef	nuint32*	pnuint32;


#define NCP_DEFAULT_BUFSIZE 	1024
#define NCP_MAX_BUFSIZE		1024
#define NCP_MAX_PACKET_SIZE 	4070
#define	NCP_MAXUSERNAMELEN	255
#define	NCP_MAXPASSWORDLEN	255
#define NCP_MAXPATHLEN		255
#define NCP_MAX_FILENAME 	14
#define NCP_FILE_ID_LEN		6

#define NCP_BINDERY_USER	0x0001
#define NCP_BINDERY_UGROUP	0x0002
#define NCP_BINDERY_PQUEUE	0x0003
#define NCP_BINDERY_FSERVER	0x0004
#define NCP_BINDERY_PSERVER	0x0007
#define NCP_BINDERY_NAME_LEN	48

/* Handle Flags */
#define	NCP_HF_DIRSHORT		0		/* short directory handle */
#define	NCP_HF_DIRBASE		1		/* directory base */
#define	NCP_HF_NONE		0xff		/* no handle or dirbase */

/* Options to negotiate */
#define	NCP_IPX_CHECKSUM		1
#define	NCP_SECURITY_LEVEL_SIGN_HEADERS	2

#ifndef NWCONN_HANDLE
#define NWCONN_HANDLE   unsigned int
#define pNWCONN_HANDLE  (unsigned int*)
#define NWCONN_NUM      u_int16_t
#define NWCCODE         unsigned int
#define NWDIR_HANDLE    u_int8_t
#define NWFILE_HANDLE   int
#endif

struct ncp_fh_s {
	u_int16_t val1; 
	union {
		u_int32_t val32;
		u_int16_t val16;
	} val;
} __packed;

typedef	struct ncp_fh_s ncp_fh;

typedef struct ncpfid_s {
	nwdirent	f_parent;
	nwdirent	f_id;
} ncpfid;

/* -- Bindery properties -- */
struct ncp_bindery_object {
	u_int32_t	object_id;
	u_int16_t	object_type;
	u_int8_t	object_name[NCP_BINDERY_NAME_LEN];
	u_int8_t	object_flags;
	u_int8_t	object_security;
	u_int8_t	object_has_prop;
};

struct nw_property {
	u_int8_t	value[128];
	u_int8_t	more_flag;
	u_int8_t	property_flag;
};

struct ncp_filesearch_info {
	u_int8_t	volume_number;
	u_int16_t	directory_id;
	u_int16_t	sequence_no;
	u_int8_t	access_rights;
};


struct ncp_file_info {
	u_int8_t	file_id[NCP_FILE_ID_LEN];
	char		file_name[NCP_MAX_FILENAME + 1];
	u_int8_t	file_attributes;
	u_int8_t	file_mode;
	u_int32_t	file_length;
	u_int16_t	creation_date;
	u_int16_t	access_date;
	u_int16_t	update_date;
	u_int16_t	update_time;
};

struct nw_queue_job_entry {
	u_int16_t	InUse;
	u_int32_t	prev;
	u_int32_t	next;
	u_int32_t	ClientStation;
	u_int32_t	ClientTask;
	u_int32_t	ClientObjectID;
	u_int32_t	TargetServerID;
	u_int8_t	TargetExecTime[6];
	u_int8_t	JobEntryTime[6];
	u_int32_t	JobNumber;
	u_int16_t	JobType;
	u_int16_t	JobPosition;
	u_int16_t	JobControlFlags;
	u_int8_t	FileNameLen;
	char		JobFileName[13];
	u_int32_t	JobFileHandle;
	u_int32_t	ServerStation;
	u_int32_t	ServerTaskNumber;
	u_int32_t	ServerObjectID;
	char		JobTextDescription[50];
	char		ClientRecordArea[152];
} __packed;

struct queue_job {
	struct nw_queue_job_entry j;
	ncp_fh file_handle;
};

#define QJE_OPER_HOLD	0x80
#define QJE_USER_HOLD	0x40
#define QJE_ENTRYOPEN	0x20
#define QJE_SERV_RESTART    0x10
#define QJE_SERV_AUTO	    0x08

/* ClientRecordArea for print jobs */

#define   KEEP_ON        0x0400
#define   NO_FORM_FEED   0x0800
#define   NOTIFICATION   0x1000
#define   DELETE_FILE    0x2000
#define   EXPAND_TABS    0x4000
#define   PRINT_BANNER   0x8000

struct print_job_record {
	u_int8_t	Version;
	u_int8_t	TabSize;
	u_int16_t	Copies;
	u_int16_t	CtrlFlags;
	u_int16_t	Lines;
	u_int16_t	Rows;
	char		FormName[16];
	u_int8_t	Reserved[6];
	char		BannerName[13];
	char		FnameBanner[13];
	char		FnameHeader[14];
	char		Path[80];
} __packed;

struct ncp_station_addr {
	u_int32_t	NetWork;
	u_int8_t	Node[6];
	u_int16_t	Socket;
} __packed;

struct ncp_prop_login_control {
	u_int8_t	AccountExpireDate[3];
	u_int8_t	Disabled;
	u_int8_t	PasswordExpireDate[3];
	u_int8_t	GraceLogins;
	u_int16_t	PasswordExpireInterval;
	u_int8_t	MaxGraceLogins;
	u_int8_t	MinPasswordLength;
	u_int16_t	MaxConnections;
	u_int8_t	ConnectionTimeMask[42];
	u_int8_t	LastLogin[6];
	u_int8_t	RestrictionMask;
	u_int8_t	reserved;
	u_int32_t	MaxDiskUsage;
	u_int16_t	BadLoginCount;
	u_int32_t	BadLoginCountDown;
	struct ncp_station_addr LastIntruder;
} __packed;

#define NCP_VOLNAME_LEN (16)
#define NCP_NUMBER_OF_VOLUMES (64)
struct ncp_volume_info {
	u_int32_t total_blocks;
	u_int32_t free_blocks;
	u_int32_t purgeable_blocks;
	u_int32_t not_yet_purgeable_blocks;
	u_int32_t total_dir_entries;
	u_int32_t available_dir_entries;
	u_int8_t sectors_per_block;
	char volume_name[NCP_VOLNAME_LEN + 1];
};
/*
 * Name space constants, taken from NDK
 */
#define aRONLY     (ntohl(0x01000000))
#define aHIDDEN    (ntohl(0x02000000))
#define aSYSTEM    (ntohl(0x04000000))
#define aEXECUTE   (ntohl(0x08000000))
#define aDIR       (ntohl(0x10000000))
#define aARCH      (ntohl(0x20000000))

/* Defines for Name Spaces */
#define NW_NS_DOS     0
#define NW_NS_MAC     1
#define NW_NS_NFS     2
#define NW_NS_FTAM    3
#define NW_NS_OS2     4

/* for _ScanNSEntryInfo */
#define	IM_NAME			0x00000001
#define	IM_SPACE_ALLOCATED	0x00000002
#define IM_ATTRIBUTES		0x00000004
#define IM_SIZE			0x00000008
#define IM_TOTAL_SIZE		0x00000010
#define IM_EA			0x00000020
#define IM_ARCHIVE		0x00000040
#define IM_MODIFY		0x00000080
#define IM_CREATION		0x00000100
#define IM_OWNING_NAMESPACE	0x00000200
#define IM_DIRECTORY		0x00000400
#define IM_RIGHTS		0x00000800
#define	IM_ALMOST_ALL		0x00000FED
#define IM_ALL			0x00000FFF
#define	IM_REFERENCE_ID		0x00001000
#define	IM_NS_ATTRIBUTES	0x00002000
#define IM_COMPRESSED_INFO	0x80000000UL

/* open/create modes */
#define OC_MODE_OPEN		0x01
#define OC_MODE_TRUNCATE	0x02
#define OC_MODE_REPLACE		0x02
#define OC_MODE_CREATE		0x08

/* open/create results */
#define OC_ACTION_NONE		0x00
#define OC_ACTION_OPEN		0x01
#define OC_ACTION_CREATE	0x02
#define OC_ACTION_TRUNCATE	0x04
#define OC_ACTION_REPLACE	0x04

/* renameFlag in NSRename */
#define	NW_TYPE_FILE		0x8000
#define	NW_TYPE_SUBDIR		0x0010

#define	NW_NAME_CONVERT		0x0003	/* don't report error and set comp mode */
#define	NW_NO_NAME_CONVERT	0x0004	/* only in specified name space */

/* search attributes */
#ifndef SA_HIDDEN
#define SA_NORMAL         0x0000
#define SA_HIDDEN         0x0002
#define SA_SYSTEM         0x0004
#define SA_SUBDIR_ONLY    0x0010
#define SA_SUBDIR_FILES   0x8000
#define SA_ALL            0x8006
#endif

/* access rights attributes */
#ifndef AR_READ
#define AR_READ			0x0001
#define AR_WRITE		0x0002
#define AR_READ_ONLY		0x0001
#define AR_WRITE_ONLY		0x0002
#define AR_DENY_READ		0x0004
#define AR_DENY_WRITE		0x0008
#define AR_COMPATIBILITY	0x0010
#define AR_WRITE_THROUGH	0x0040
#define AR_OPEN_COMPRESSED	0x0100
#endif

struct nw_entry_info {
	u_int32_t	spaceAlloc;
	u_int32_t	attributes;	/* LH */
	u_int16_t	flags;		/* internal */
	u_int32_t	dataStreamSize;
	u_int32_t	totalStreamSize;
	u_int16_t	numberOfStreams;
	u_int16_t	creationTime;	/* LH */
	u_int16_t	creationDate;	/* LH */
	u_int32_t	creatorID;	/* HL */
	u_int16_t	modifyTime;	/* LH */
	u_int16_t	modifyDate;	/* LH */
	u_int32_t	modifierID;	/* HL */
	u_int16_t	lastAccessDate;	/* LH */
	u_int16_t	archiveTime;	/* LH */
	u_int16_t	archiveDate;	/* LH */
	u_int32_t	archiverID;	/* HL */
	u_int16_t	inheritedRightsMask;	/* LH */
	u_int32_t	dirEntNum;
	u_int32_t	DosDirNum;
	u_int32_t	volNumber;
	u_int32_t	EADataSize;
	u_int32_t	EAKeyCount;
	u_int32_t	EAKeySize;
	u_int32_t	NSCreator;
	u_int8_t	nameLen;
	u_int8_t	entryName[256];
} __packed;

typedef struct nw_entry_info NW_ENTRY_INFO;

/* modify mask - use with MODIFY_DOS_INFO structure */
#define DM_ATTRIBUTES		0x0002L
#define DM_CREATE_DATE		0x0004L
#define DM_CREATE_TIME		0x0008L
#define DM_CREATOR_ID		0x0010L
#define DM_ARCHIVE_DATE		0x0020L
#define DM_ARCHIVE_TIME		0x0040L
#define DM_ARCHIVER_ID		0x0080L
#define DM_MODIFY_DATE		0x0100L
#define DM_MODIFY_TIME		0x0200L
#define DM_MODIFIER_ID		0x0400L
#define DM_LAST_ACCESS_DATE	0x0800L
#define DM_INHERITED_RIGHTS_MASK	0x1000L
#define DM_MAXIMUM_SPACE	0x2000L

struct nw_modify_dos_info {
	u_int32_t attributes;
	u_int16_t creationDate;
	u_int16_t creationTime;
	u_int32_t creatorID;
	u_int16_t modifyDate;
	u_int16_t modifyTime;
	u_int32_t modifierID;
	u_int16_t archiveDate;
	u_int16_t archiveTime;
	u_int32_t archiverID;
	u_int16_t lastAccessDate;
	u_int16_t inheritanceGrantMask;
	u_int16_t inheritanceRevokeMask;
	u_int32_t maximumSpace;
}  __packed;

struct nw_search_seq {
	u_int8_t	volNumber;
	u_int32_t	dirNumber;
	u_int32_t	searchDirNumber;
}  __packed;

typedef struct nw_search_seq SEARCH_SEQUENCE;

struct ncp_file_server_info {
	u_int8_t	ServerName[48];
	u_int8_t	FileServiceVersion;
	u_int8_t	FileServiceSubVersion;
	u_int16_t	MaximumServiceConnections;
	u_int16_t	ConnectionsInUse;
	u_int16_t	NumberMountedVolumes;
	u_int8_t	Revision;
	u_int8_t	SFTLevel;
	u_int8_t	TTSLevel;
	u_int16_t	MaxConnectionsEverUsed;
	u_int8_t	AccountVersion;
	u_int8_t	VAPVersion;
	u_int8_t	QueueVersion;
	u_int8_t	PrintVersion;
	u_int8_t	VirtualConsoleVersion;
	u_int8_t	RestrictionLevel;
	u_int8_t	InternetBridge;
	u_int8_t	Reserved[60];
} __packed;

struct nw_time_buffer {
	u_int8_t	year;
	u_int8_t	month;
	u_int8_t	day;
	u_int8_t	hour;
	u_int8_t	minute;
	u_int8_t	second;
	u_int8_t	wday;
} __packed;

#endif /*_NCP_H_ */
