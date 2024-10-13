import numpy
from _typeshed import Incomplete
from typing import ClassVar, overload

ASYNC_ENCODE_SUPPORT: NV_ENC_CAPS
BGR: PixelFormat
BIT_DEPTH_NOT_SUPPORTED: TaskExecInfo
BT_601: ColorSpace
BT_709: ColorSpace
DEBUG: FfmpegLogLevel
DYNAMIC_QUERY_ENCODER_CAPACITY: NV_ENC_CAPS
END_OF_STREAM: TaskExecInfo
ERROR: FfmpegLogLevel
EXPOSED_COUNT: NV_ENC_CAPS
FAIL: TaskExecInfo
FATAL: FfmpegLogLevel
HEIGHT_MAX: NV_ENC_CAPS
HEIGHT_MIN: NV_ENC_CAPS
INFO: FfmpegLogLevel
INVALID_INPUT: TaskExecInfo
JPEG: ColorRange
LEVEL_MAX: NV_ENC_CAPS
LEVEL_MIN: NV_ENC_CAPS
MB_NUM_MAX: NV_ENC_CAPS
MB_PER_SEC_MAX: NV_ENC_CAPS
MORE_DATA_NEEDED: TaskExecInfo
MPEG: ColorRange
NOT_SUPPORTED: TaskExecInfo
NO_PTS: int
NUM_MAX_BFRAMES: NV_ENC_CAPS
NUM_MAX_LTR_FRAMES: NV_ENC_CAPS
NUM_MAX_TEMPORAL_LAYERS: NV_ENC_CAPS
NV12: PixelFormat
P10: PixelFormat
P12: PixelFormat
PANIC: FfmpegLogLevel
PREPROC_SUPPORT: NV_ENC_CAPS
RES_CHANGE: TaskExecInfo
RGB: PixelFormat
RGB_32F: PixelFormat
RGB_32F_PLANAR: PixelFormat
RGB_PLANAR: PixelFormat
SEPARATE_COLOUR_PLANE: NV_ENC_CAPS
SRC_DST_SIZE_MISMATCH: TaskExecInfo
SUCCESS: TaskExecInfo
SUPPORTED_RATECONTROL_MODES: NV_ENC_CAPS
SUPPORT_10BIT_ENCODE: NV_ENC_CAPS
SUPPORT_ADAPTIVE_TRANSFORM: NV_ENC_CAPS
SUPPORT_ALPHA_LAYER_ENCODING: NV_ENC_CAPS
SUPPORT_BDIRECT_MODE: NV_ENC_CAPS
SUPPORT_BFRAME_REF_MODE: NV_ENC_CAPS
SUPPORT_CABAC: NV_ENC_CAPS
SUPPORT_CONSTRAINED_ENCODING: NV_ENC_CAPS
SUPPORT_CUSTOM_VBV_BUF_SIZE: NV_ENC_CAPS
SUPPORT_DYNAMIC_SLICE_MODE: NV_ENC_CAPS
SUPPORT_DYN_BITRATE_CHANGE: NV_ENC_CAPS
SUPPORT_DYN_FORCE_CONSTQP: NV_ENC_CAPS
SUPPORT_DYN_RCMODE_CHANGE: NV_ENC_CAPS
SUPPORT_DYN_RES_CHANGE: NV_ENC_CAPS
SUPPORT_EMPHASIS_LEVEL_MAP: NV_ENC_CAPS
SUPPORT_FIELD_ENCODING: NV_ENC_CAPS
SUPPORT_FMO: NV_ENC_CAPS
SUPPORT_HIERARCHICAL_BFRAMES: NV_ENC_CAPS
SUPPORT_HIERARCHICAL_PFRAMES: NV_ENC_CAPS
SUPPORT_INTRA_REFRESH: NV_ENC_CAPS
SUPPORT_LOOKAHEAD: NV_ENC_CAPS
SUPPORT_LOSSLESS_ENCODE: NV_ENC_CAPS
SUPPORT_MEONLY_MODE: NV_ENC_CAPS
SUPPORT_MONOCHROME: NV_ENC_CAPS
SUPPORT_MULTIPLE_REF_FRAMES: NV_ENC_CAPS
SUPPORT_QPELMV: NV_ENC_CAPS
SUPPORT_REF_PIC_INVALIDATION: NV_ENC_CAPS
SUPPORT_SAO: NV_ENC_CAPS
SUPPORT_STEREO_MVC: NV_ENC_CAPS
SUPPORT_SUBFRAME_READBACK: NV_ENC_CAPS
SUPPORT_TEMPORAL_AQ: NV_ENC_CAPS
SUPPORT_TEMPORAL_SVC: NV_ENC_CAPS
SUPPORT_WEIGHTED_PREDICTION: NV_ENC_CAPS
SUPPORT_YUV444_ENCODE: NV_ENC_CAPS
UDEF: ColorRange
UNDEFINED: PixelFormat
UNSPEC: ColorSpace
UNSUPPORTED_FMT_CONV_PARAMS: TaskExecInfo
VERBOSE: FfmpegLogLevel
WARNING: FfmpegLogLevel
WIDTH_MAX: NV_ENC_CAPS
WIDTH_MIN: NV_ENC_CAPS
Y: PixelFormat
YUV420: PixelFormat
YUV420_10bit: PixelFormat
YUV422: PixelFormat
YUV444: PixelFormat
YUV444_10bit: PixelFormat
kDLCPU: DLDeviceType
kDLCUDA: DLDeviceType
kDLCUDAHost: DLDeviceType
kDLCUDAManaged: DLDeviceType

class ColorRange:
    __members__: ClassVar[dict] = ...  # read-only
    JPEG: ClassVar[ColorRange] = ...
    MPEG: ClassVar[ColorRange] = ...
    UDEF: ClassVar[ColorRange] = ...
    __entries: ClassVar[dict] = ...
    def __init__(self, value: int) -> None: ...
    def __eq__(self, other: object) -> bool: ...
    def __hash__(self) -> int: ...
    def __index__(self) -> int: ...
    def __int__(self) -> int: ...
    def __ne__(self, other: object) -> bool: ...
    @property
    def name(self) -> str: ...
    @property
    def value(self) -> int: ...

class ColorSpace:
    __members__: ClassVar[dict] = ...  # read-only
    BT_601: ClassVar[ColorSpace] = ...
    BT_709: ClassVar[ColorSpace] = ...
    UNSPEC: ClassVar[ColorSpace] = ...
    __entries: ClassVar[dict] = ...
    def __init__(self, value: int) -> None: ...
    def __eq__(self, other: object) -> bool: ...
    def __hash__(self) -> int: ...
    def __index__(self) -> int: ...
    def __int__(self) -> int: ...
    def __ne__(self, other: object) -> bool: ...
    @property
    def name(self) -> str: ...
    @property
    def value(self) -> int: ...

class ColorspaceConversionContext:
    color_range: ColorRange
    color_space: ColorSpace
    @overload
    def __init__(self) -> None: ...
    @overload
    def __init__(self, color_space: ColorSpace, color_range: ColorRange) -> None: ...

class CudaBuffer:
    def __init__(self, *args, **kwargs) -> None: ...
    def Clone(self) -> CudaBuffer: ...
    @overload
    def CopyFrom(self, other: CudaBuffer, stream: int) -> None: ...
    @overload
    def CopyFrom(self, other: CudaBuffer, gpu_id: int) -> None: ...
    @staticmethod
    def Make(elem_size: int, num_elems: int, gpu_id: int) -> CudaBuffer: ...
    @property
    def ElemSize(self) -> int: ...
    @property
    def GpuMem(self) -> int: ...
    @property
    def NumElems(self) -> int: ...
    @property
    def RawMemSize(self) -> int: ...

class DLDeviceType:
    __members__: ClassVar[dict] = ...  # read-only
    __entries: ClassVar[dict] = ...
    kDLCPU: ClassVar[DLDeviceType] = ...
    kDLCUDA: ClassVar[DLDeviceType] = ...
    kDLCUDAHost: ClassVar[DLDeviceType] = ...
    kDLCUDAManaged: ClassVar[DLDeviceType] = ...
    def __init__(self, value: int) -> None: ...
    def __eq__(self, other: object) -> bool: ...
    def __hash__(self) -> int: ...
    def __index__(self) -> int: ...
    def __int__(self) -> int: ...
    def __ne__(self, other: object) -> bool: ...
    @property
    def name(self) -> str: ...
    @property
    def value(self) -> int: ...

class FfmpegLogLevel:
    __members__: ClassVar[dict] = ...  # read-only
    DEBUG: ClassVar[FfmpegLogLevel] = ...
    ERROR: ClassVar[FfmpegLogLevel] = ...
    FATAL: ClassVar[FfmpegLogLevel] = ...
    INFO: ClassVar[FfmpegLogLevel] = ...
    PANIC: ClassVar[FfmpegLogLevel] = ...
    VERBOSE: ClassVar[FfmpegLogLevel] = ...
    WARNING: ClassVar[FfmpegLogLevel] = ...
    __entries: ClassVar[dict] = ...
    def __init__(self, value: int) -> None: ...
    def __eq__(self, other: object) -> bool: ...
    def __hash__(self) -> int: ...
    def __index__(self) -> int: ...
    def __int__(self) -> int: ...
    def __ne__(self, other: object) -> bool: ...
    @property
    def name(self) -> str: ...
    @property
    def value(self) -> int: ...

class MotionVector:
    dst_x: int
    dst_y: int
    h: int
    motion_scale: int
    motion_x: int
    motion_y: int
    source: int
    src_x: int
    src_y: int
    w: int
    def __init__(self) -> None: ...

class NV_ENC_CAPS:
    __members__: ClassVar[dict] = ...  # read-only
    ASYNC_ENCODE_SUPPORT: ClassVar[NV_ENC_CAPS] = ...
    DYNAMIC_QUERY_ENCODER_CAPACITY: ClassVar[NV_ENC_CAPS] = ...
    EXPOSED_COUNT: ClassVar[NV_ENC_CAPS] = ...
    HEIGHT_MAX: ClassVar[NV_ENC_CAPS] = ...
    HEIGHT_MIN: ClassVar[NV_ENC_CAPS] = ...
    LEVEL_MAX: ClassVar[NV_ENC_CAPS] = ...
    LEVEL_MIN: ClassVar[NV_ENC_CAPS] = ...
    MB_NUM_MAX: ClassVar[NV_ENC_CAPS] = ...
    MB_PER_SEC_MAX: ClassVar[NV_ENC_CAPS] = ...
    NUM_MAX_BFRAMES: ClassVar[NV_ENC_CAPS] = ...
    NUM_MAX_LTR_FRAMES: ClassVar[NV_ENC_CAPS] = ...
    NUM_MAX_TEMPORAL_LAYERS: ClassVar[NV_ENC_CAPS] = ...
    PREPROC_SUPPORT: ClassVar[NV_ENC_CAPS] = ...
    SEPARATE_COLOUR_PLANE: ClassVar[NV_ENC_CAPS] = ...
    SUPPORTED_RATECONTROL_MODES: ClassVar[NV_ENC_CAPS] = ...
    SUPPORT_10BIT_ENCODE: ClassVar[NV_ENC_CAPS] = ...
    SUPPORT_ADAPTIVE_TRANSFORM: ClassVar[NV_ENC_CAPS] = ...
    SUPPORT_ALPHA_LAYER_ENCODING: ClassVar[NV_ENC_CAPS] = ...
    SUPPORT_BDIRECT_MODE: ClassVar[NV_ENC_CAPS] = ...
    SUPPORT_BFRAME_REF_MODE: ClassVar[NV_ENC_CAPS] = ...
    SUPPORT_CABAC: ClassVar[NV_ENC_CAPS] = ...
    SUPPORT_CONSTRAINED_ENCODING: ClassVar[NV_ENC_CAPS] = ...
    SUPPORT_CUSTOM_VBV_BUF_SIZE: ClassVar[NV_ENC_CAPS] = ...
    SUPPORT_DYNAMIC_SLICE_MODE: ClassVar[NV_ENC_CAPS] = ...
    SUPPORT_DYN_BITRATE_CHANGE: ClassVar[NV_ENC_CAPS] = ...
    SUPPORT_DYN_FORCE_CONSTQP: ClassVar[NV_ENC_CAPS] = ...
    SUPPORT_DYN_RCMODE_CHANGE: ClassVar[NV_ENC_CAPS] = ...
    SUPPORT_DYN_RES_CHANGE: ClassVar[NV_ENC_CAPS] = ...
    SUPPORT_EMPHASIS_LEVEL_MAP: ClassVar[NV_ENC_CAPS] = ...
    SUPPORT_FIELD_ENCODING: ClassVar[NV_ENC_CAPS] = ...
    SUPPORT_FMO: ClassVar[NV_ENC_CAPS] = ...
    SUPPORT_HIERARCHICAL_BFRAMES: ClassVar[NV_ENC_CAPS] = ...
    SUPPORT_HIERARCHICAL_PFRAMES: ClassVar[NV_ENC_CAPS] = ...
    SUPPORT_INTRA_REFRESH: ClassVar[NV_ENC_CAPS] = ...
    SUPPORT_LOOKAHEAD: ClassVar[NV_ENC_CAPS] = ...
    SUPPORT_LOSSLESS_ENCODE: ClassVar[NV_ENC_CAPS] = ...
    SUPPORT_MEONLY_MODE: ClassVar[NV_ENC_CAPS] = ...
    SUPPORT_MONOCHROME: ClassVar[NV_ENC_CAPS] = ...
    SUPPORT_MULTIPLE_REF_FRAMES: ClassVar[NV_ENC_CAPS] = ...
    SUPPORT_QPELMV: ClassVar[NV_ENC_CAPS] = ...
    SUPPORT_REF_PIC_INVALIDATION: ClassVar[NV_ENC_CAPS] = ...
    SUPPORT_SAO: ClassVar[NV_ENC_CAPS] = ...
    SUPPORT_STEREO_MVC: ClassVar[NV_ENC_CAPS] = ...
    SUPPORT_SUBFRAME_READBACK: ClassVar[NV_ENC_CAPS] = ...
    SUPPORT_TEMPORAL_AQ: ClassVar[NV_ENC_CAPS] = ...
    SUPPORT_TEMPORAL_SVC: ClassVar[NV_ENC_CAPS] = ...
    SUPPORT_WEIGHTED_PREDICTION: ClassVar[NV_ENC_CAPS] = ...
    SUPPORT_YUV444_ENCODE: ClassVar[NV_ENC_CAPS] = ...
    WIDTH_MAX: ClassVar[NV_ENC_CAPS] = ...
    WIDTH_MIN: ClassVar[NV_ENC_CAPS] = ...
    __entries: ClassVar[dict] = ...
    def __init__(self, value: int) -> None: ...
    def __eq__(self, other: object) -> bool: ...
    def __hash__(self) -> int: ...
    def __index__(self) -> int: ...
    def __int__(self) -> int: ...
    def __ne__(self, other: object) -> bool: ...
    @property
    def name(self) -> str: ...
    @property
    def value(self) -> int: ...

class NvJpegEncodeContext:
    def __init__(self, *args, **kwargs) -> None: ...
    def Compression(self) -> int: ...
    def Format(self) -> PixelFormat: ...

class PacketData:
    bsl: int
    dts: int
    duration: int
    key: int
    pos: int
    pts: int
    def __init__(self) -> None: ...

class PixelFormat:
    __members__: ClassVar[dict] = ...  # read-only
    BGR: ClassVar[PixelFormat] = ...
    NV12: ClassVar[PixelFormat] = ...
    P10: ClassVar[PixelFormat] = ...
    P12: ClassVar[PixelFormat] = ...
    RGB: ClassVar[PixelFormat] = ...
    RGB_32F: ClassVar[PixelFormat] = ...
    RGB_32F_PLANAR: ClassVar[PixelFormat] = ...
    RGB_PLANAR: ClassVar[PixelFormat] = ...
    UNDEFINED: ClassVar[PixelFormat] = ...
    Y: ClassVar[PixelFormat] = ...
    YUV420: ClassVar[PixelFormat] = ...
    YUV420_10bit: ClassVar[PixelFormat] = ...
    YUV422: ClassVar[PixelFormat] = ...
    YUV444: ClassVar[PixelFormat] = ...
    YUV444_10bit: ClassVar[PixelFormat] = ...
    __entries: ClassVar[dict] = ...
    def __init__(self, value: int) -> None: ...
    def __eq__(self, other: object) -> bool: ...
    def __hash__(self) -> int: ...
    def __index__(self) -> int: ...
    def __int__(self) -> int: ...
    def __ne__(self, other: object) -> bool: ...
    @property
    def name(self) -> str: ...
    @property
    def value(self) -> int: ...

class PyDecoder:
    def __init__(self, input: str, opts: dict[str, str], gpu_id: int = ...) -> None: ...
    @overload
    def DecodeSingleFrame(self, frame: numpy.ndarray, seek_ctx: SeekContext | None = ...) -> tuple[bool, TaskExecInfo]: ...
    @overload
    def DecodeSingleFrame(self, frame: numpy.ndarray, pkt_data: PacketData, seek_ctx: SeekContext | None = ...) -> tuple[bool, TaskExecInfo]: ...
    @overload
    def DecodeSingleSurface(self, surf, seek_ctx: SeekContext | None = ...) -> tuple[bool, TaskExecInfo]: ...
    @overload
    def DecodeSingleSurface(self, surf, pkt_data: PacketData, seek_ctx: SeekContext | None = ...) -> tuple[bool, TaskExecInfo]: ...
    @property
    def AvgFramerate(self) -> float: ...
    @property
    def Bitrate(self) -> int: ...
    @property
    def ColorRange(self) -> ColorRange: ...
    @property
    def ColorSpace(self) -> ColorSpace: ...
    @property
    def Delay(self) -> int: ...
    @property
    def Duration(self) -> float: ...
    @property
    def Format(self) -> PixelFormat: ...
    @property
    def Framerate(self) -> float: ...
    @property
    def GopSize(self) -> int: ...
    @property
    def Height(self) -> int: ...
    @property
    def HostFrameSize(self) -> int: ...
    @property
    def IsAccelerated(self) -> bool: ...
    @property
    def IsVFR(self) -> bool: ...
    @property
    def Level(self) -> int: ...
    @property
    def Metadata(self) -> dict[str, str]: ...
    @property
    def MotionVectors(self) -> list[MotionVector]: ...
    @property
    def NumFrames(self) -> int: ...
    @property
    def NumStreams(self) -> int: ...
    @property
    def Profile(self) -> int: ...
    @property
    def StartTime(self) -> float: ...
    @property
    def StreamIndex(self) -> int: ...
    @property
    def Timebase(self) -> float: ...
    @property
    def Width(self) -> int: ...

class PyFrameConverter:
    def __init__(self, width: int, height: int, src_format: PixelFormat, dst_format: PixelFormat) -> None: ...
    def Run(self, src: numpy.ndarray, dst: numpy.ndarray, cc_ctx: ColorspaceConversionContext) -> tuple[bool, TaskExecInfo]: ...
    @property
    def Format(self) -> PixelFormat: ...

class PyFrameUploader:
    @overload
    def __init__(self, gpu_id: int) -> None: ...
    @overload
    def __init__(self, stream: int) -> None: ...
    def Run(self, src: numpy.ndarray, dst) -> tuple[bool, TaskExecInfo]: ...

class PyNvEncoder:
    @overload
    def __init__(self, settings: dict[str, str], gpu_id: int, format: PixelFormat = ..., verbose: bool = ...) -> None: ...
    @overload
    def __init__(self, settings: dict[str, str], stream: int, format: PixelFormat = ..., verbose: bool = ...) -> None: ...
    @overload
    def EncodeSingleSurface(self, surface, packet: numpy.ndarray[numpy.uint8], sei: numpy.ndarray[numpy.uint8], sync: bool, append: bool) -> bool: ...
    @overload
    def EncodeSingleSurface(self, surface, packet: numpy.ndarray[numpy.uint8], sei: numpy.ndarray[numpy.uint8], sync: bool) -> bool: ...
    @overload
    def EncodeSingleSurface(self, surface, packet: numpy.ndarray[numpy.uint8], sync: bool) -> bool: ...
    @overload
    def EncodeSingleSurface(self, surface, packet: numpy.ndarray[numpy.uint8], sei: numpy.ndarray[numpy.uint8]) -> bool: ...
    @overload
    def EncodeSingleSurface(self, surface, packet: numpy.ndarray[numpy.uint8]) -> bool: ...
    def Flush(self, packets: numpy.ndarray[numpy.uint8]) -> bool: ...
    def FlushSinglePacket(self, packets: numpy.ndarray[numpy.uint8]) -> bool: ...
    def Reconfigure(self, settings: dict[str, str], force_idr: bool = ..., reset_encoder: bool = ..., verbose: bool = ...) -> bool: ...
    @property
    def Capabilities(self) -> dict[NV_ENC_CAPS, int]: ...
    @property
    def Format(self) -> PixelFormat: ...
    @property
    def FrameSizeInBytes(self) -> int: ...
    @property
    def Height(self) -> int: ...
    @property
    def Width(self) -> int: ...

class PyNvJpegEncoder:
    def __init__(self, gpu_id: int) -> None: ...
    def Context(self, compression: int, pixel_format: PixelFormat) -> NvJpegEncodeContext: ...
    def Run(self, context: NvJpegEncodeContext, surfaces: list[Surface]) -> tuple[list[numpy.ndarray], TaskExecInfo]: ...

class PySurfaceConverter:
    @overload
    def __init__(self, src_format: PixelFormat, dst_format: PixelFormat, gpu_id: int) -> None: ...
    @overload
    def __init__(self, src_format: PixelFormat, dst_format: PixelFormat, stream: int) -> None: ...
    def Run(self, src, dst, cc_ctx: ColorspaceConversionContext | None = ...) -> tuple[bool, TaskExecInfo]: ...

class PySurfaceDownloader:
    @overload
    def __init__(self, gpu_id: int) -> None: ...
    @overload
    def __init__(self, stream: int) -> None: ...
    def Run(self, src, dst: numpy.ndarray) -> tuple[bool, TaskExecInfo]: ...

class PySurfaceResizer:
    @overload
    def __init__(self, format: PixelFormat, gpu_id: int) -> None: ...
    @overload
    def __init__(self, format: PixelFormat, stream: int) -> None: ...
    def Run(self, src, dst) -> tuple[bool, TaskExecInfo]: ...

class SeekContext:
    seek_frame: int
    seek_tssec: float
    @overload
    def __init__(self, seek_frame: int) -> None: ...
    @overload
    def __init__(self, seek_ts: float) -> None: ...

class Surface:
    def __init__(self, *args, **kwargs) -> None: ...
    def Clone(self) -> Surface: ...
    @overload
    @staticmethod
    def Make(format: PixelFormat, width: int, height: int, gpu_id: int) -> Surface: ...
    @overload
    @staticmethod
    def Make(format: PixelFormat, width: int, height: int, context: int) -> Surface: ...
    @staticmethod
    def from_cai(dict: object, format: PixelFormat = ...) -> Surface: ...
    @staticmethod
    def from_dlpack(capsule: capsule, format: PixelFormat = ...) -> Surface: ...
    def __dlpack__(self, stream: int = ...) -> capsule: ...
    def __dlpack_device__(self) -> tuple[DLDeviceType, int]: ...
    @property
    def Format(self) -> PixelFormat: ...
    @property
    def Height(self) -> int: ...
    @property
    def HostSize(self) -> int: ...
    @property
    def IsEmpty(self) -> bool: ...
    @property
    def IsOwnMemory(self) -> bool: ...
    @property
    def NumPlanes(self) -> int: ...
    @property
    def Pitch(self) -> int: ...
    @property
    def Planes(self) -> tuple: ...
    @property
    def Width(self) -> int: ...
    @property
    def __cuda_array_interface__(self) -> dict: ...

class SurfacePlane:
    def __init__(self, *args, **kwargs) -> None: ...
    def __dlpack__(self, stream: int = ...) -> capsule: ...
    def __dlpack_device__(self) -> tuple[DLDeviceType, int]: ...
    @property
    def ElemSize(self) -> int: ...
    @property
    def GpuMem(self) -> int: ...
    @property
    def Height(self) -> int: ...
    @property
    def HostFrameSize(self) -> int: ...
    @property
    def Pitch(self) -> int: ...
    @property
    def Width(self) -> int: ...
    @property
    def __cuda_array_interface__(self) -> dict: ...

class TaskExecDetails:
    info: TaskExecInfo
    message: str
    status: Incomplete
    def __init__(self) -> None: ...

class TaskExecInfo:
    __members__: ClassVar[dict] = ...  # read-only
    BIT_DEPTH_NOT_SUPPORTED: ClassVar[TaskExecInfo] = ...
    END_OF_STREAM: ClassVar[TaskExecInfo] = ...
    FAIL: ClassVar[TaskExecInfo] = ...
    INVALID_INPUT: ClassVar[TaskExecInfo] = ...
    MORE_DATA_NEEDED: ClassVar[TaskExecInfo] = ...
    NOT_SUPPORTED: ClassVar[TaskExecInfo] = ...
    RES_CHANGE: ClassVar[TaskExecInfo] = ...
    SRC_DST_SIZE_MISMATCH: ClassVar[TaskExecInfo] = ...
    SUCCESS: ClassVar[TaskExecInfo] = ...
    UNSUPPORTED_FMT_CONV_PARAMS: ClassVar[TaskExecInfo] = ...
    __entries: ClassVar[dict] = ...
    def __init__(self, value: int) -> None: ...
    def __eq__(self, other: object) -> bool: ...
    def __hash__(self) -> int: ...
    def __index__(self) -> int: ...
    def __int__(self) -> int: ...
    def __ne__(self, other: object) -> bool: ...
    @property
    def name(self) -> str: ...
    @property
    def value(self) -> int: ...

def GetNumGpus() -> int: ...
def GetNvencParams() -> dict[str, str]: ...
def SetFFMpegLogLevel(arg0: FfmpegLogLevel) -> None: ...
