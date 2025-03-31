package com.nearlink.demo.dd;

import android.util.Log;

import com.nearlink.demo.UILog;

import java.io.IOException;
import java.io.PrintWriter;
import java.io.StringWriter;
import java.lang.annotation.Annotation;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Set;

public class DdTestEntry{

    private static final String TAG = "DdTestEntry";

    Map<String, Map<String, ReflectInfo>> testUnitMap = new HashMap<>();

    /**
     * 初始化测试用例
     */
    public void init(){
        loadClass("com.nearlink.demo.dd.EnableTest");
        loadClass("com.nearlink.demo.dd.AnnounceTest");
        loadClass("com.nearlink.demo.dd.DiscoveryTest");
        loadClass("com.nearlink.demo.dd.SeekTest");
        loadClass("com.nearlink.demo.dd.DeviceLocalNameTest");
        loadClass("com.nearlink.demo.dd.AddressTest");
        loadClass("com.nearlink.demo.dd.DiscoveryTestV2");
        loadClass("com.nearlink.demo.dd.RemoteDeviceTest");
    }

    /**
     * 根据模块名称和测试用例执行测试
     *
     * @param moduleName
     * @param testMethodName
     */
    public void execute(String moduleName, String testMethodName) {
        try {
            asynExecute(moduleName, testMethodName);
        } catch (Exception e) {
            Log.e(TAG, errInfo(e));
        }

    }

    static class ReflectInfo {

        Method method;
        Class clazz;

        public ReflectInfo(Method method, Class clazz) {
            this.method = method;
            this.clazz = clazz;
        }
    }

    private void loadClass(String classPath) {

        Class clazz = null;
        try {
            clazz = Class.forName(classPath);
        } catch (ClassNotFoundException e) {
            throw new NearlinkTestException("找不到测试类" + classPath);
        }

        if (!clazz.isAnnotationPresent(NearlinkTestClass.class)) {
            throw new NearlinkTestException("测试类" + classPath + "没有NearlinkTestClass注解");
        }

        Annotation nearlinkTestClassAnnotation = clazz.getAnnotation(NearlinkTestClass.class);

        Method nearlinkTestClassAnnotationValueMethod = null;
        try {
            nearlinkTestClassAnnotationValueMethod = nearlinkTestClassAnnotation.getClass().getDeclaredMethod("value"
                    , new Class[0]);
        } catch (NoSuchMethodException e) {
            throw new NearlinkTestException("注解" + nearlinkTestClassAnnotation.toString() + "没有value" + e.getMessage());
        }

        String moduleName = null;
        try {
            moduleName = (String) nearlinkTestClassAnnotationValueMethod.invoke(nearlinkTestClassAnnotation,
                    new Object[]{});
        } catch (InvocationTargetException | IllegalAccessException e) {
            throw new NearlinkTestException("反射获取@NearlinkTestClass.value值异常 " + e.getMessage());
        }


        /**
         * moduleExist (key:methodName - value:ReflectInfo)
         */
        Map<String, ReflectInfo> moduleExist = testUnitMap.get(moduleName);
        if (moduleExist == null) {
            moduleExist = new HashMap<>();
            testUnitMap.put(moduleName, moduleExist);
        }

        Method[] methods = clazz.getMethods();
        for (Method method : methods) {

            Annotation annotationNearlinkTest = method.getAnnotation(NearlinkTest.class);

            if (annotationNearlinkTest == null) {
                continue;
            }

            Method annotationNearlinkTestValueMethod = null;

            try {
                annotationNearlinkTestValueMethod = annotationNearlinkTest.getClass().getDeclaredMethod("value",
                        new Class[0]);
            } catch (NoSuchMethodException e) {
                throw new NearlinkTestException(annotationNearlinkTest.toString() + "value不存在" + e.getMessage());
            }


            String methodName = null;

            try {
                methodName = (String) annotationNearlinkTestValueMethod.invoke(annotationNearlinkTest, new Object[]{});
            } catch (InvocationTargetException | IllegalAccessException e) {
                throw new NearlinkTestException("反射获取@NearlinkTest.value异常 " + e.getMessage());
            }


            ReflectInfo existMethodTestInfo = moduleExist.get(methodName);
            if (existMethodTestInfo == null) {
                existMethodTestInfo = new ReflectInfo(method, clazz);
                moduleExist.put(methodName, existMethodTestInfo);
            }
        }
    }

    private void executeTestMethod(String testMethodName, ReflectInfo reflectInfo) throws Exception {

        UILog.log(UILog.FLAG_MAIN, "=== start excute " + testMethodName + "=====>");
        Object instance = reflectInfo.clazz.newInstance();
        Method beforeTestMethod = reflectInfo.clazz.getMethod("beforeTestMethod");
        Method afterTestMethod = reflectInfo.clazz.getMethod("afterTestMethod");
        Method testMethod = reflectInfo.method;

        beforeTestMethod.invoke(instance, new Object[] {});
        testMethod.invoke(instance, new Object[] {});
        afterTestMethod.invoke(instance, new Object[] {});

        UILog.log(UILog.FLAG_MAIN, "<=== end excute " + testMethodName + "=====");
    }

    private void asynExecute(String moduleName, String testMethodName) throws Exception {

        Thread thread = new Thread(() -> {

            if (DdUtils.isBlank(moduleName)) {
                UILog.log(UILog.FLAG_MAIN, "moduleName为空");
                return;
            }

            Map<String, ReflectInfo> testMethodsMap = testUnitMap.get(moduleName);

            if (testMethodsMap == null) {
                UILog.log(UILog.FLAG_MAIN, moduleName + "找不到对应的测试模块");
                return;
            }

            if (DdUtils.isBlank(testMethodName)) {

                Set<Map.Entry<String, ReflectInfo>> entrySet = testMethodsMap.entrySet();
                Iterator<Map.Entry<String, ReflectInfo>> it = entrySet.iterator();
                while (it.hasNext()) {
                    Map.Entry<String, ReflectInfo> entry = it.next();
                    try {
                        executeTestMethod(entry.getKey(), entry.getValue());
                    } catch (Exception e) {
                        Log.e(TAG, errInfo(e));
                    }

                }

                return;
            }

            ReflectInfo reflectInfo = testMethodsMap.get(testMethodName);
            if (reflectInfo == null) {
                UILog.log(UILog.FLAG_MAIN, "测试模块" + moduleName + "找不到测试函数：" + testMethodName);
                return;
            }

            try {
                executeTestMethod(testMethodName, reflectInfo);
            } catch (Exception e) {
                Log.e(TAG, errInfo(e));
            }
        });
        thread.start();

    }


    public static String errInfo(Exception e) {
        StringWriter sw = null;
        PrintWriter pw = null;
        try {
            sw = new StringWriter();
            pw = new PrintWriter(sw);
            // 将出错的栈信息输出到printWriter中
            e.printStackTrace(pw);
            pw.flush();
            sw.flush();
        } finally {
            if (sw != null) {
                try {
                    sw.close();
                } catch (IOException e1) {
                    e1.printStackTrace();
                }
            }
            if (pw != null) {
                pw.close();
            }
        }
        return sw.toString();
    }

    class NearlinkTestException extends RuntimeException {

        public NearlinkTestException(String message) {
            super(message);
        }
    }
}
