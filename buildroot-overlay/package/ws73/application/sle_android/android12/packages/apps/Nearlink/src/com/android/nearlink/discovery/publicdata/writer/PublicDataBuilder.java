/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.nearlink.discovery.publicdata.writer;

import android.util.Log;
import com.android.nearlink.discovery.publicdata.PublicDataEnums;
import com.android.nearlink.discovery.publicdata.writer.impl.data.impl.BytePublicDataWriter;
import com.android.nearlink.discovery.publicdata.writer.impl.data.impl.StringPublicDataWriter;

import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;

/**
 * 广播公开数据构造器
 *
 * @since 2023-12-04
 */
public class PublicDataBuilder {

    private static final String TAG = "PublicDataBuilder";

    private static final Map<Integer, ISubPublicDataWriter> subPublicDataWriterMapper = new HashMap<>();

    static {
        initSubPublicDataWriterMapper(subPublicDataWriterMapper);
    }

    private static void initSubPublicDataWriterMapper(Map<Integer, ISubPublicDataWriter> subPublicDataWriterMapper){


        subPublicDataWriterMapper.put(PublicDataEnums.SEEK_LEVEL.getTypeVal(),
                new BytePublicDataWriter());

        subPublicDataWriterMapper.put(PublicDataEnums.ACCESS_LAYER_CAPABILITY.getTypeVal(),
                new BytePublicDataWriter());

        subPublicDataWriterMapper.put(PublicDataEnums.DEVICE_SHORT_LOCAL_NAME.getTypeVal(),
                new StringPublicDataWriter());

        subPublicDataWriterMapper.put(PublicDataEnums.ANNOUNCE_TX_POWER.getTypeVal(),
                new BytePublicDataWriter());
    }


    public static Build builder(){
        return new Build();
    }


    public static class Build {

        private PublicData publicData = new PublicData(0, new byte[0]);

        public <T> Build writeValue(T value, PublicDataEnums publicDataEnums){
            return this.writeValue(true, value, publicDataEnums);
        }

        public <T> Build writeValue(boolean flag, T value, PublicDataEnums publicDataEnums){

            if(!flag){
                Log.e(TAG, "writeValue() flag is false");
                return this;
            }

            ISubPublicDataWriter iSubPublicDataWriter = subPublicDataWriterMapper
                    .get(publicDataEnums.getTypeVal());

            if(iSubPublicDataWriter == null){
                Log.e(TAG, "error! type=" + publicDataEnums.getTypeVal() + " no any ISubPublicDataWriter match");
                return this;
            }

            byte[] tmp = iSubPublicDataWriter.writeValue(value, publicDataEnums);

            int len = publicData.getLength() + tmp.length;

            publicData.setLength(len);

            byte[] old = publicData.getData();
            if(old == null){
                publicData.setData(tmp);
            }else {
                byte[] res = concat(old, tmp);
                publicData.setData(res);
            }

            return this;

        }

        public PublicData build(){
            return this.publicData;
        }

        private byte[] concat(byte[] byte1, byte[] byte2){
            byte[] res = Arrays.copyOf(byte1, byte1.length + byte2.length);
            System.arraycopy(byte2, 0, res, byte1.length, byte2.length);
            return res;
        }

    }




}
