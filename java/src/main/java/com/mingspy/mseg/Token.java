package com.mingspy.mseg;

public class Token
{
    public int start;
    public int end;
    public String nature;
    public String word;
    //public int attr;
    public Token(int start, int end)
    {
        this.start = start;
        this.end = end;
    }
    public Token(int start, int end, String nature)
    {
        this.start = start;
        this.end = end;
        this.nature = nature;
    }
    @Override
    public String toString()
    {
        StringBuilder builder = new StringBuilder();
        builder.append("(");
        builder.append(start);
        builder.append(",");
        builder.append(end);
        if(word != null) {
            builder.append(word);
        }
        if(nature != null) {
            builder.append("/");
            builder.append(nature);
        }
        builder.append(")");
        return builder.toString();
    }

}
