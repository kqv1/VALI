import numpy
from typing import ClassVar, overload

ASYNC_ENCODE_SUPPORT: NV_ENC_CAPS
AV1: CudaVideoCodec
BGR: PixelFormat
BIT_DEPTH_NOT_SUPPORTED: TaskExecInfo
BT_601: ColorSpace
BT_709: ColorSpace
DYNAMIC_QUERY_ENCODER_CAPACITY: NV_ENC_CAPS
END_OF_STREAM: TaskExecInfo
EXACT_FRAME: SeekMode
EXPOSED_COUNT: NV_ENC_CAPS
FAIL: TaskExecInfo
H264: CudaVideoCodec
HEIGHT_MAX: NV_ENC_CAPS
HEIGHT_MIN: NV_ENC_CAPS
HEVC: CudaVideoCodec
INVALID_INPUT: TaskExecInfo
JPEG: ColorRange
LEVEL_MAX: NV_ENC_CAPS
LEVEL_MIN: NV_ENC_CAPS
MB_NUM_MAX: NV_ENC_CAPS
MB_PER_SEC_MAX: NV_ENC_CAPS
MJPEG: CudaVideoCodec
MORE_DATA_NEEDED: TaskExecInfo
MPEG: ColorRange
MPEG1: CudaVideoCodec
MPEG2: CudaVideoCodec
MPEG4: CudaVideoCodec
NUM_MAX_BFRAMES: NV_ENC_CAPS
NUM_MAX_LTR_FRAMES: NV_ENC_CAPS
NUM_MAX_TEMPORAL_LAYERS: NV_ENC_CAPS
NV12: PixelFormat
P10: PixelFormat
P12: PixelFormat
PREPROC_SUPPORT: NV_ENC_CAPS
PREV_KEY_FRAME: SeekMode
RGB: PixelFormat
RGB_32F: PixelFormat
RGB_32F_PLANAR: PixelFormat
RGB_PLANAR: PixelFormat
SEPARATE_COLOUR_PLANE: NV_ENC_CAPS
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
VC1: CudaVideoCodec
VP9: CudaVideoCodec
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
    def CopyFrom(self, other: CudaBuffer, context: int, stream: int) -> None: ...
    @overload
    def CopyFrom(self, other: CudaBuffer, gpu_id: int) -> None: ...
    def GetElemSize(self) -> int: ...
    def GetNumElems(self) -> int: ...
    def GetRawMemSize(self) -> int: ...
    def GpuMem(self) -> int: ...
    @staticmethod
    def Make(elem_size: int, num_elems: int, gpu_id: int) -> CudaBuffer: ...

class CudaVideoCodec:
    __members__: ClassVar[dict] = ...  # read-only
    AV1: ClassVar[CudaVideoCodec] = ...
    H264: ClassVar[CudaVideoCodec] = ...
    HEVC: ClassVar[CudaVideoCodec] = ...
    MJPEG: ClassVar[CudaVideoCodec] = ...
    MPEG1: ClassVar[CudaVideoCodec] = ...
    MPEG2: ClassVar[CudaVideoCodec] = ...
    MPEG4: ClassVar[CudaVideoCodec] = ...
    VC1: ClassVar[CudaVideoCodec] = ...
    VP9: ClassVar[CudaVideoCodec] = ...
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

class CuvidParserException(Exception): ...

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

class HwResetException(Exception): ...

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
    def Accelerated(self) -> bool: ...
    def AvgFramerate(self) -> float: ...
    def Codec(self) -> CudaVideoCodec: ...
    def ColorRange(self) -> ColorRange: ...
    def ColorSpace(self) -> ColorSpace: ...
    @overload
    def DecodeSingleFrame(self, frame: numpy.ndarray) -> tuple[bool, TaskExecInfo]: ...
    @overload
    def DecodeSingleFrame(self, frame: numpy.ndarray, pktData: PacketData) -> tuple[bool, TaskExecInfo]: ...
    @overload
    def DecodeSingleSurface(self, surf) -> tuple[bool, TaskExecInfo]: ...
    @overload
    def DecodeSingleSurface(self, surf, pktData: PacketData) -> tuple[bool, TaskExecInfo]: ...
    def Format(self) -> PixelFormat: ...
    def Framerate(self) -> float: ...
    def GetMotionVectors(self, *args, **kwargs): ...
    def Height(self) -> int: ...
    def Numframes(self) -> int: ...
    def Timebase(self) -> float: ...
    def Width(self) -> int: ...

class PyFrameConverter:
    def __init__(self, width: int, height: int, src_format: PixelFormat, dst_format: PixelFormat) -> None: ...
    def Format(self) -> PixelFormat: ...
    def Run(self, src: numpy.ndarray, dst: numpy.ndarray, cc_ctx: ColorspaceConversionContext) -> tuple[bool, TaskExecInfo]: ...

class PyFrameUploader:
    @overload
    def __init__(self, gpu_id: int) -> None: ...
    @overload
    def __init__(self, stream: int) -> None: ...
    def Run(self, src: numpy.ndarray, dst) -> bool: ...

class PyNvEncoder:
    @overload
    def __init__(self, settings: dict[str, str], gpu_id: int, format: PixelFormat = ..., verbose: bool = ...) -> None: ...
    @overload
    def __init__(self, settings: dict[str, str], context: int, stream: int, format: PixelFormat = ..., verbose: bool = ...) -> None: ...
    def Capabilities(self) -> dict[NV_ENC_CAPS, int]: ...
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
    def Format(self) -> PixelFormat: ...
    def GetFrameSizeInBytes(self) -> int: ...
    def Height(self) -> int: ...
    def Reconfigure(self, settings: dict[str, str], force_idr: bool = ..., reset_encoder: bool = ..., verbose: bool = ...) -> bool: ...
    def Width(self) -> int: ...

class PySurfaceConverter:
    @overload
    def __init__(self, src_format: PixelFormat, dst_format: PixelFormat, gpu_id: int) -> None: ...
    @overload
    def __init__(self, src_format: PixelFormat, dst_format: PixelFormat, stream: int) -> None: ...
    def Run(self, src, dst, cc_ctx: ColorspaceConversionContext) -> tuple[bool, TaskExecInfo]: ...

class PySurfaceDownloader:
    @overload
    def __init__(self, gpu_id: int) -> None: ...
    @overload
    def __init__(self, stream: int) -> None: ...
    def Run(self, src, dst: numpy.ndarray) -> bool: ...

class PySurfaceResizer:
    @overload
    def __init__(self, format: PixelFormat, gpu_id: int) -> None: ...
    @overload
    def __init__(self, format: PixelFormat, stream: int) -> None: ...
    def Run(self, src, dst) -> bool: ...

class SeekContext:
    mode: SeekMode
    out_frame_pts: int
    seek_frame: int
    seek_tssec: float
    @overload
    def __init__(self, seek_frame: int) -> None: ...
    @overload
    def __init__(self, seek_frame: int, mode: SeekMode) -> None: ...
    @overload
    def __init__(self, seek_ts: float) -> None: ...
    @overload
    def __init__(self, seek_ts: float, mode: SeekMode) -> None: ...
    @property
    def num_frames_decoded(self) -> int: ...

class SeekMode:
    __members__: ClassVar[dict] = ...  # read-only
    EXACT_FRAME: ClassVar[SeekMode] = ...
    PREV_KEY_FRAME: ClassVar[SeekMode] = ...
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

class Surface:
    def __init__(self, *args, **kwargs) -> None: ...
    def Clone(self) -> Surface: ...
    def Empty(self) -> bool: ...
    def Format(self) -> PixelFormat: ...
    def Height(self, plane: int = ...) -> int: ...
    def HostSize(self) -> int: ...
    @overload
    @staticmethod
    def Make(format: PixelFormat, width: int, height: int, gpu_id: int) -> Surface: ...
    @overload
    @staticmethod
    def Make(format: PixelFormat, width: int, height: int, context: int) -> Surface: ...
    def NumPlanes(self) -> int: ...
    def OwnMemory(self) -> bool: ...
    def Pitch(self, plane: int = ...) -> int: ...
    def PlanePtr(self, plane: int = ...) -> SurfacePlane: ...
    def Width(self, plane: int = ...) -> int: ...
    @staticmethod
    def from_dlpack(capsule: capsule, format: PixelFormat = ...) -> Surface: ...
    def __dlpack__(self, stream: int = ...) -> capsule: ...
    def __dlpack_device__(self) -> tuple[DLDeviceType, int]: ...

class SurfacePlane:
    def __init__(self, *args, **kwargs) -> None: ...
    def ElemSize(self) -> int: ...
    def Height(self) -> int: ...
    def HostFrameSize(self) -> int: ...
    def Pitch(self) -> int: ...
    def Width(self) -> int: ...
    def __dlpack__(self, stream: int = ...) -> capsule: ...
    def __dlpack_device__(self) -> tuple[DLDeviceType, int]: ...

class TaskExecDetails:
    info: TaskExecInfo
    def __init__(self) -> None: ...

class TaskExecInfo:
    __members__: ClassVar[dict] = ...  # read-only
    BIT_DEPTH_NOT_SUPPORTED: ClassVar[TaskExecInfo] = ...
    END_OF_STREAM: ClassVar[TaskExecInfo] = ...
    FAIL: ClassVar[TaskExecInfo] = ...
    INVALID_INPUT: ClassVar[TaskExecInfo] = ...
    MORE_DATA_NEEDED: ClassVar[TaskExecInfo] = ...
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
