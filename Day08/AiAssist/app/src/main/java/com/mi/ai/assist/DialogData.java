package com.mi.ai.assist;

enum FromType {
    me,
    ai
}

public class DialogData {
    private String content;  // 用于存储 question 或 answer
    private long timestamp;
    private FromType fromType;
    private int type = 0;
    
    // 无参构造函数
    public DialogData() {
        this("", 0);
    }
    
    // 带参数的构造函数
    public DialogData(String content, long timestamp) {
        this.content = content;
        this.timestamp = timestamp;
    }
    
    // Getter 和 Setter 方法
    public String getContent() {
        return content;
    }
    
    public void setContent(String content) {
        this.content = content;
    }
    
    public long getTimestamp() {
        return timestamp;
    }
    
    public void setTimestamp(long timestamp) {
        this.timestamp = timestamp;
    }
    
    public FromType getFromType() {
        return fromType;
    }
    
    public void setFromType(FromType fromType) {
        this.fromType = fromType;
    }
    
    public int getType() {
        return type;
    }
    
    public void setType(int type) {
        this.type = type;
    }
}

class AnswerData {
    public String answer = "";
    public float time = 0;
}

class QuestionData {
    public String question = "";
    public long time = 0;
}
