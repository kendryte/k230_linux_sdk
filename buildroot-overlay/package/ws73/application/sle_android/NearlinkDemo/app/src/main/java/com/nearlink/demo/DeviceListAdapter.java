package com.nearlink.demo;

import java.util.List;

import android.content.Context;
import android.nearlink.NearlinkConstant;
import android.text.TextUtils;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.TextView;

import com.nearlink.demo.data.DeviceInfo;

public class DeviceListAdapter extends ArrayAdapter<DeviceInfo> {
    public DeviceListAdapter(Context context, int resource, List<DeviceInfo> objects) {
        super(context, resource, objects);
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        DeviceInfo device = getItem(position);
        View view;
        //新增一个内部类 ViewHolder，用于对控件的实例进行缓存
        ViewHolder viewHolder;
        if (convertView == null) {
            //为每一个子项加载设定的布局
            view = LayoutInflater.from(getContext()).inflate(R.layout.deviceitem_layout, parent, false);
            viewHolder = new ViewHolder();
            //分别获取 imageview 和 textview 的实例
            viewHolder.deviceName = view.findViewById(R.id.device_name);
            viewHolder.deviceAddress = view.findViewById(R.id.device_address);
            viewHolder.deviceRssi = view.findViewById(R.id.device_rssi);
            viewHolder.devicePairState = view.findViewById(R.id.device_pairState);
            viewHolder.deviceConnState = view.findViewById(R.id.device_connState);
            view.setTag(viewHolder);//将 viewHolder 存储在 view 中
        } else {
            view = convertView;
            viewHolder = (ViewHolder) view.getTag();//重新获取 viewHolder
        }
        CharSequence text = viewHolder.deviceName.getText();
        if (text != null) {
            String oldName = text.toString();
            if (TextUtils.isEmpty(oldName)) {
                viewHolder.deviceName.setText(device.name);
            } else if (!oldName.equals(device.name)) {
                viewHolder.deviceName.setText(device.name);
            }
        } else {
            viewHolder.deviceName.setText(device.name);
        }

        viewHolder.deviceAddress.setText(device.address);
        viewHolder.deviceRssi.setText(String.valueOf(device.rssi));
        viewHolder.devicePairState.setText(getPairStateName(device.pairState));
        viewHolder.deviceConnState.setText(getConnStateName(device.connState));
        return view;
    }


    private class ViewHolder {
        TextView deviceName;
        TextView deviceAddress;
        TextView deviceRssi;
        TextView devicePairState;
        TextView deviceConnState;
    }

    private String getPairStateName(int pairState) {
        switch (pairState) {
            case NearlinkConstant.SLE_PAIR_NONE:
                return "未配对";
            case NearlinkConstant.SLE_PAIR_PAIRING:
                return "配对中";
            case NearlinkConstant.SLE_PAIR_PAIRED:
                return "已配对";
            case NearlinkConstant.SLE_PAIR_REMOVING:
                return "取消配对中";
            default:
                return "未知";

        }
    }

    private String getConnStateName(int connState) {
        switch (connState) {
            case NearlinkConstant.SLE_ACB_STATE_NONE:
                return "未连接";
            case NearlinkConstant.SLE_ACB_STATE_CONNECTED:
                return "已连接";
            case NearlinkConstant.SLE_ACB_STATE_DISCONNECTED:
                return "已断连";
            case NearlinkConstant.SLE_ACB_STATE_CONNECTING:
                return "连接中";
            case NearlinkConstant.SLE_ACB_STATE_DISCONNECTING:
                return "断连中";
            default:
                return "未知";

        }
    }
}
