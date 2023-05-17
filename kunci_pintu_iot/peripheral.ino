void servo_open(void)
{
    digitalWrite(error_status, HIGH);
}

void servo_close(void)
{
    digitalWrite(error_status, LOW);
}

