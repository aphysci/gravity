classdef Protobuf < handle
    methods (Abstract)
        builder = getProtobufBuilder(this);
    end
end